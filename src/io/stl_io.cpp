#include "stl_io.hpp"
#include "texture.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

// ---------------------------------------------------------------------------
// STL é um formato de malha triangular. Há duas variantes:
//   - ASCII:  texto com blocos "facet normal ... outer loop / vertex ..."
//   - Binária: cabeçalho de 80 bytes + contagem de triângulos (4 bytes) +
//              triângulos de 50 bytes cada (normal 3 floats, 3 vértices de
//              3 floats e 2 bytes de atributo).
// O carregador detecta a variante e chama a rotina correspondente.
// ---------------------------------------------------------------------------

// Lê um arquivo STL em formato ASCII, construindo a lista de triângulos.
// Linhas são limpas (tabs, CR e espaços duplicados) para robustez na leitura.
static bool load_stl_ascii(const std::string &path) {
  std::ifstream f(path.c_str());
  if (!f)
    return false;
  std::string line;
  size_t facet_count = 0;
  while (std::getline(f, line)) {
    std::string trim;
    for (size_t i = 0; i < line.size(); i++) {
      char c = line[i];
      if (c == '\t' || c == '\r')
        continue;
      if (c == ' ' && !trim.empty() && trim.back() == ' ')
        continue;
      if (c != ' ' || !trim.empty())
        trim.push_back(c);
    }
    while (!trim.empty() && trim.back() == ' ')
      trim.pop_back();

    // Para cada bloco "facet", lê a normal (se houver) e os três vértices.
    if (trim.substr(0, 12) == "facet normal" || trim.substr(0, 5) == "facet") {
      Triangle tri;
      tri.normal = Vec3(0, 0, 0);
      if (trim.substr(0, 12) == "facet normal") {
        std::stringstream ss(trim);
        std::string tok;
        ss >> tok >> tok;  // descarta "facet normal"
        ss >> tri.normal.x >> tri.normal.y >> tri.normal.z;
      }
      int vi = 0;
      while (std::getline(f, line) && vi < 3) {
        std::string t;
        for (size_t i = 0; i < line.size(); i++) {
          char c = line[i];
          if (c == '\t' || c == '\r')
            continue;
          t.push_back(c);
        }
        if (t.find("vertex") != std::string::npos) {
          std::stringstream ss(t);
          std::string tok;
          ss >> tok;  // descarta "vertex"
          ss >> tri.v[vi].x >> tri.v[vi].y >> tri.v[vi].z;
          vi++;
        }
      }
      if (vi == 3) {
        g_triangles.push_back(tri);
        facet_count++;
      }
    }
  }
  return facet_count > 0;
}

// Lê um arquivo STL binário. A estrutura fixa de cada triângulo é lida
// diretamente para o struct Triangle; normais ausentes são corrigidas depois
// em load_stl().
static bool load_stl_binary(const std::string &path) {
  std::ifstream f(path.c_str(), std::ios::binary);
  if (!f)
    return false;
  char header[80];
  f.read(header, 80);
  unsigned int count = 0;
  f.read(reinterpret_cast<char *>(&count), 4);
  for (unsigned int i = 0; i < count; i++) {
    Triangle tri;
    f.read(reinterpret_cast<char *>(&tri.normal), sizeof(float) * 3);
    f.read(reinterpret_cast<char *>(&tri.v[0]), sizeof(float) * 3);
    f.read(reinterpret_cast<char *>(&tri.v[1]), sizeof(float) * 3);
    f.read(reinterpret_cast<char *>(&tri.v[2]), sizeof(float) * 3);
    unsigned short attr;
    f.read(reinterpret_cast<char *>(&attr), 2);
    g_triangles.push_back(tri);
    if (f.eof() || f.fail())
      break;
  }
  return !g_triangles.empty();
}

// Detecta se o arquivo é STL ASCII: deve começar com "solid" e, nas linhas
// seguintes, conter "facet" ou "vertex". Isso evita interpretar um binário
// que por acaso comece com "solid".
static bool is_ascii_stl(const std::string &path) {
  std::ifstream f(path.c_str());
  if (!f)
    return false;
  std::string first;
  std::getline(f, first);
  for (char &c : first)
    c = (char)tolower(c);
  if (first.find("solid") == std::string::npos)
    return false;
  for (int i = 0; i < 10; i++) {
    std::string l;
    std::getline(f, l);
    for (char &c : l)
      c = (char)tolower(c);
    if (l.find("facet") != std::string::npos ||
        l.find("vertex") != std::string::npos)
      return true;
  }
  return false;
}

// Ponto de entrada do carregamento: limpa a malha anterior, detecta o formato
// e delega para o leitor adequado. Se o arquivo não possuir normais válidas,
// recalcula-as a partir do produto vetorial das arestas do triângulo.
bool load_stl(const std::string &path) {
  g_triangles.clear();
  g_model_loaded = false;

  bool ascii = is_ascii_stl(path);

  bool ok = false;
  if (ascii) {
    ok = load_stl_ascii(path);
    if (!ok)
      ok = load_stl_binary(path);
  } else {
    ok = load_stl_binary(path);
  }

  if (ok) {
    for (size_t i = 0; i < g_triangles.size(); i++) {
      // Se a normal não foi fornecida, calcula a partir das arestas do
      // triângulo (e1 x e2) e normaliza.
      if (g_triangles[i].normal.length() < 1e-6f) {
        Vec3 e1 = g_triangles[i].v[1] - g_triangles[i].v[0];
        Vec3 e2 = g_triangles[i].v[2] - g_triangles[i].v[0];
        Vec3 n = e1.cross(e2);
        n.normalize();
        g_triangles[i].normal = n;
      }
    }
    g_model_loaded = true;
  }
  return ok;
}

// ---------------------------------------------------------------------------
// Leitura de OBJ (Wavefront), usado para importar modelos que já trazem
// textura. Subconjunto suportado: vértices (v), normais (vn), coordenadas de
// textura (vt), faces (f) e material (mtllib/usemtl). Outras linhas (o, g, s)
// são ignoradas.
// ---------------------------------------------------------------------------

// Retorna o nome do arquivo (sem o diretório) de um caminho.
static std::string file_basename(const std::string &p) {
  size_t pos = p.find_last_of("/\\");
  if (pos == std::string::npos)
    return p;
  return p.substr(pos + 1);
}

// Retorna o diretório de um caminho ("." quando não há diretório).
static std::string file_dirname(const std::string &p) {
  size_t pos = p.find_last_of("/\\");
  if (pos == std::string::npos)
    return ".";
  return p.substr(0, pos);
}

// Troca a extensão de um caminho (ex.: modelo.obj -> modelo.mtl).
static std::string replace_extension(const std::string &p,
                                     const std::string &new_ext) {
  size_t pos = p.find_last_of('.');
  if (pos == std::string::npos)
    return p + new_ext;
  return p.substr(0, pos) + new_ext;
}

// Copia um arquivo em modo binário (usado para levar a textura para junto do
// arquivo .obj exportado).
static bool copy_file(const std::string &src, const std::string &dst) {
  std::ifstream in(src.c_str(), std::ios::binary);
  std::ofstream out(dst.c_str(), std::ios::binary);
  if (!in || !out)
    return false;
  out << in.rdbuf();
  return out.good();
}

// Converte a extensão de um caminho para minúsculas (".OBJ" -> ".obj").
static std::string lower_extension(const std::string &p) {
  size_t pos = p.find_last_of('.');
  if (pos == std::string::npos)
    return "";
  std::string e = p.substr(pos);
  for (size_t i = 0; i < e.size(); i++)
    e[i] = (char)tolower((unsigned char)e[i]);
  return e;
}

// Converte uma string em inteiro com verificação de sucesso (strtol): retorna
// 0 quando nenhum dígito é encontrado, em vez de valor indefinido.
static int parse_int(const std::string &s) {
  const char *c = s.c_str();
  char *end = nullptr;
  long v = strtol(c, &end, 10);
  return (end == c) ? 0 : (int)v;
}

// Lê um índice de face "v", "v/t", "v//n" ou "v/t/n". Retorna 0 quando a
// parte correspondente não existe na face.
static void parse_face_index(const std::string &comp, int &vi, int &ti,
                             int &ni) {
  vi = ti = ni = 0;
  size_t p1 = comp.find('/');
  if (p1 == std::string::npos) {
    vi = parse_int(comp);
    return;
  }
  vi = parse_int(comp.substr(0, p1));
  size_t p2 = comp.find('/', p1 + 1);
  if (p2 == std::string::npos) {
    ti = parse_int(comp.substr(p1 + 1));
    return;
  }
  if (p2 > p1 + 1)
    ti = parse_int(comp.substr(p1 + 1, p2 - p1 - 1));
  ni = parse_int(comp.substr(p2 + 1));
}

// Resolve um índice de vértice do OBJ (1-baseado ou negativo relativo) para
// um índice de vetor válido; retorna -1 se fora do intervalo.
static int resolve_index(int idx, int count) {
  int real = (idx > 0) ? idx - 1 : count + idx;
  return (real >= 0 && real < count) ? real : -1;
}

// Procura o arquivo de textura (map_Kd) dentro de um arquivo .mtl.
static std::string read_mtl_texture(const std::string &mtl_path) {
  std::ifstream f(mtl_path.c_str());
  if (!f)
    return "";
  std::string line;
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    std::string tok;
    if (!(ss >> tok))
      continue;
    if (tok == "map_Kd") {
      std::string tex;
      if (ss >> tex)
        return tex;
    }
  }
  return "";
}

// Carrega um arquivo OBJ para g_triangles, com vértices, normais, UVs e, se o
// .mtl referenciar uma textura (map_Kd), carrega essa textura. O caminho da
// textura no .mtl é relativo ao próprio arquivo .mtl.
bool load_obj(const std::string &path) {
  std::ifstream f(path.c_str());
  if (!f)
    return false;

  std::vector<Vec3> verts, norms;
  std::vector<Vec2> uvs;
  std::string mtl_file;
  g_triangles.clear();
  g_model_loaded = false;

  std::string line;
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    std::string tok;
    if (!(ss >> tok) || tok[0] == '#')
      continue;

    if (tok == "v") {
      Vec3 v;
      ss >> v.x >> v.y >> v.z;
      verts.push_back(v);
    } else if (tok == "vn") {
      Vec3 n;
      ss >> n.x >> n.y >> n.z;
      norms.push_back(n);
    } else if (tok == "vt") {
      float u = 0.0f, v = 0.0f;
      ss >> u >> v;
      uvs.push_back(Vec2(u, v));
    } else if (tok == "mtllib") {
      std::string m;
      ss >> m;
      mtl_file = m;  // apenas o primeiro é usado
    } else if (tok == "f") {
      // Coleta os índices de vértice/textura/normal de cada componente.
      std::vector<int> fv, ft, fn;
      std::string comp;
      while (ss >> comp) {
        int vi = 0, ti = 0, ni = 0;
        parse_face_index(comp, vi, ti, ni);
        fv.push_back(vi);
        ft.push_back(ti);
        fn.push_back(ni);
      }
      // Triangulação em leque a partir do primeiro vértice da face.
      for (size_t i = 1; i + 1 < fv.size(); i++) {
        int order[3] = {0, (int)i, (int)i + 1};
        Triangle tri;
        bool has_normal = false;
        for (int k = 0; k < 3; k++) {
          int vp = resolve_index(fv[order[k]], (int)verts.size());
          tri.v[k] = (vp >= 0) ? verts[vp] : Vec3();
          int tp = ft[order[k]];
          int up = resolve_index(tp, (int)uvs.size());
          tri.uv[k] = (tp != 0 && up >= 0) ? uvs[up] : Vec2();
          int np = fn[order[k]];
          int rp = resolve_index(np, (int)norms.size());
          if (np != 0 && rp >= 0) {
            tri.normal = norms[rp];
            has_normal = true;
          }
        }
        if (!has_normal)
          tri.normal = Vec3(0, 0, 0);
        g_triangles.push_back(tri);
      }
    }
    // o, g, s e usemtl são ignorados (não afetam a geometria).
  }

  if (g_triangles.empty())
    return false;

  // Recalcula normais ausentes a partir das arestas do triângulo.
  for (size_t i = 0; i < g_triangles.size(); i++) {
    if (g_triangles[i].normal.length() < 1e-6f) {
      Vec3 e1 = g_triangles[i].v[1] - g_triangles[i].v[0];
      Vec3 e2 = g_triangles[i].v[2] - g_triangles[i].v[0];
      Vec3 n = e1.cross(e2);
      n.normalize();
      g_triangles[i].normal = n;
    }
  }

  g_model_loaded = true;

  // Se o OBJ referenciar material com textura, carrega a textura. O caminho é
  // resolvido em relação ao diretório do arquivo .mtl.
  if (!mtl_file.empty()) {
    std::string mtl_path = file_dirname(path) + "/" + mtl_file;
    std::string tex_file = read_mtl_texture(mtl_path);
    if (!tex_file.empty()) {
      std::string tex_path = file_dirname(mtl_path) + "/" + tex_file;
      load_texture(tex_path);
    }
  }
  return true;
}

// Carrega um modelo pelo formato (pela extensão do arquivo):
//   - .obj: mantém orientação e UVs do arquivo e carrega a textura referenciada;
//   - outros (STL): orienta para o padrão da cena (Z->Y, frente para +Z) e gera
//     coordenadas de textura, pois o STL não armazena UVs.
// A centralização é feita à parte, por center_model(), após esta chamada.
bool load_model(const std::string &path) {
  if (lower_extension(path) == ".obj")
    return load_obj(path);
  bool ok = load_stl(path);
  if (ok) {
    orient_model();
    generate_uv_coords();
  }
  return ok;
}

// Exporta a malha atual para um arquivo STL ASCII.
// Aplica a escala atual (feita via matriz na renderização) aos vértices
// exportados, para que o arquivo reflita o modelo como exibido na tela.
bool export_stl_ascii(const std::string &path) {
  std::ofstream f(path.c_str());
  if (!f)
    return false;
  f << "solid exported\n";
  for (size_t i = 0; i < g_triangles.size(); i++) {
    Vec3 v[3];
    for (int j = 0; j < 3; j++) {
      v[j] = Vec3(g_triangles[i].v[j].x * g_scale_x,
                  g_triangles[i].v[j].y * g_scale_y,
                  g_triangles[i].v[j].z * g_scale_z);
    }
    Vec3 n = (v[1] - v[0]).cross(v[2] - v[0]);
    n.normalize();
    f << "  facet normal " << n.x << " " << n.y << " " << n.z << "\n";
    f << "    outer loop\n";
    for (int j = 0; j < 3; j++)
      f << "      vertex " << v[j].x << " " << v[j].y << " " << v[j].z << "\n";
    f << "    endloop\n";
    f << "  endfacet\n";
  }
  f << "endsolid exported\n";
  return true;
}

// Exporta a malha atual como Wavefront OBJ com textura. O STL não suporta
// textura, então quando o modelo está texturizado este formato é usado:
//   - modelo.obj: geometria (vértices, normais, UVs) e faces com o material;
//   - modelo.mtl: material que referencia a textura (map_Kd);
//   - a textura original é copiada para o mesmo diretório do .obj.
// Assim como o STL, aplica a escala atual aos vértices.
bool export_obj(const std::string &path) {
  std::string mtl_path = replace_extension(path, ".mtl");
  std::string tex_name = file_basename(g_texture_path);
  std::string tex_dst = file_dirname(path) + "/" + tex_name;

  // Escreve o arquivo .mtl com o material referenciando a textura.
  {
    std::ofstream f(mtl_path.c_str());
    if (!f)
      return false;
    f << "newmtl texture\n";
    f << "Ka 0.2 0.2 0.2\n";
    f << "Kd 1.0 1.0 1.0\n";
    f << "Ks 0.3 0.3 0.3\n";
    f << "Ns 30.0\n";
    f << "d 1.0\n";
    f << "illum 2\n";
    f << "map_Kd " << tex_name << "\n";
  }

  // Copia a textura para o diretório do .obj, para o material funcionar em
  // outros programas. Se já estiver lá, não recopia.
  if (g_texture_path != tex_dst && !copy_file(g_texture_path, tex_dst)) {
    printf("Aviso: nao foi possivel copiar a textura %s para %s\n",
           g_texture_path.c_str(), tex_dst.c_str());
  }

  std::ofstream f(path.c_str());
  if (!f)
    return false;
  f << "mtllib " << file_basename(mtl_path) << "\n";
  f << "o modelo\n";
  f << "usemtl texture\n";

  // 1. Vértices (com a escala aplicada).
  for (size_t i = 0; i < g_triangles.size(); i++) {
    for (int j = 0; j < 3; j++) {
      Vec3 v(g_triangles[i].v[j].x * g_scale_x,
             g_triangles[i].v[j].y * g_scale_y,
             g_triangles[i].v[j].z * g_scale_z);
      f << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }
  }

  // 2. Normais recalculadas a partir dos vértices escalados.
  for (size_t i = 0; i < g_triangles.size(); i++) {
    Vec3 v[3];
    for (int j = 0; j < 3; j++) {
      v[j] = Vec3(g_triangles[i].v[j].x * g_scale_x,
                  g_triangles[i].v[j].y * g_scale_y,
                  g_triangles[i].v[j].z * g_scale_z);
    }
    Vec3 n = (v[1] - v[0]).cross(v[2] - v[0]);
    n.normalize();
    f << "vn " << n.x << " " << n.y << " " << n.z << "\n";
  }

  // 3. Coordenadas de textura.
  for (size_t i = 0; i < g_triangles.size(); i++) {
    for (int j = 0; j < 3; j++)
      f << "vt " << g_triangles[i].uv[j].u << " "
        << g_triangles[i].uv[j].v << "\n";
  }

  // 4. Faces, com índices 1-baseados de vértice/textura/normal.
  //    Cada triângulo ocupa 3 vértices consecutivos e 1 normal.
  for (size_t i = 0; i < g_triangles.size(); i++) {
    int base = (int)i * 3 + 1;
    f << "f " << base << "/" << base << "/" << (i + 1) << " "
      << (base + 1) << "/" << (base + 1) << "/" << (i + 1) << " "
      << (base + 2) << "/" << (base + 2) << "/" << (i + 1) << "\n";
  }
  return true;
}

// Centraliza o modelo na origem e o normaliza. Primeiro calcula a caixa
// envolvente (bounding box) de todos os vértices, depois translada cada
// vértice pelo centro e divide pelo maior eixo. Isso garante que qualquer
// modelo, independente do tamanho original, ocupe uma área similar na tela.
void center_model() {
  if (g_triangles.empty())
    return;
  Vec3 minv, maxv;
  minv = maxv = g_triangles[0].v[0];
  for (size_t i = 0; i < g_triangles.size(); i++) {
    for (int j = 0; j < 3; j++) {
      minv.x = std::min(minv.x, g_triangles[i].v[j].x);
      minv.y = std::min(minv.y, g_triangles[i].v[j].y);
      minv.z = std::min(minv.z, g_triangles[i].v[j].z);
      maxv.x = std::max(maxv.x, g_triangles[i].v[j].x);
      maxv.y = std::max(maxv.y, g_triangles[i].v[j].y);
      maxv.z = std::max(maxv.z, g_triangles[i].v[j].z);
    }
  }
  Vec3 center = (minv + maxv) * 0.5f;
  float diag = (maxv - minv).length();
  if (diag < 1e-8f)
    diag = 1.0f;
  for (size_t i = 0; i < g_triangles.size(); i++) {
    for (int j = 0; j < 3; j++) {
      g_triangles[i].v[j].x = (g_triangles[i].v[j].x - center.x) / diag * 2.0f;
      g_triangles[i].v[j].y = (g_triangles[i].v[j].y - center.y) / diag * 2.0f;
      g_triangles[i].v[j].z = (g_triangles[i].v[j].z - center.z) / diag * 2.0f;
    }
  }
}

// Arquivos STL (CAD/impressão 3D) usam Z como eixo vertical, mas a cena usa Y.
// Rotaciona o modelo -90 graus ao redor de X: leva Z para Y (modelo em pé) e a
// frente do modelo (que no arquivo aponta para -Y) para o eixo +Z (em direção
// à câmera). Aplica-se aos vértices e também às normais, para manter a
// iluminação correta.
void orient_model() {
  for (size_t i = 0; i < g_triangles.size(); i++) {
    for (int j = 0; j < 3; j++) {
      float x = g_triangles[i].v[j].x;
      float y = g_triangles[i].v[j].y;
      float z = g_triangles[i].v[j].z;
      g_triangles[i].v[j].x = x;
      g_triangles[i].v[j].y = z;
      g_triangles[i].v[j].z = -y;
    }
    float nx = g_triangles[i].normal.x;
    float ny = g_triangles[i].normal.y;
    float nz = g_triangles[i].normal.z;
    g_triangles[i].normal.x = nx;
    g_triangles[i].normal.y = nz;
    g_triangles[i].normal.z = -ny;
  }
}

// Gera coordenadas de textura (u, v) por vértice. As coordenadas são derivadas
// da posição do vértice dentro da caixa envolvente do modelo, normalizadas
// pelo maior eixo, o que mapeia a textura de forma estável sobre a superfície
// independentemente da escala do modelo.
void generate_uv_coords() {
  if (g_triangles.empty())
    return;
  Vec3 minv, maxv;
  minv = maxv = g_triangles[0].v[0];
  for (size_t i = 0; i < g_triangles.size(); i++) {
    for (int j = 0; j < 3; j++) {
      minv.x = std::min(minv.x, g_triangles[i].v[j].x);
      minv.y = std::min(minv.y, g_triangles[i].v[j].y);
      minv.z = std::min(minv.z, g_triangles[i].v[j].z);
      maxv.x = std::max(maxv.x, g_triangles[i].v[j].x);
      maxv.y = std::max(maxv.y, g_triangles[i].v[j].y);
      maxv.z = std::max(maxv.z, g_triangles[i].v[j].z);
    }
  }
  float dx = maxv.x - minv.x;
  float dy = maxv.y - minv.y;
  float dz = maxv.z - minv.z;
  float max_range = std::max({dx, dy, dz});
  if (max_range < 1e-8f)
    max_range = 1.0f;

  for (size_t i = 0; i < g_triangles.size(); i++) {
    for (int j = 0; j < 3; j++) {
      g_triangles[i].uv[j].u = (g_triangles[i].v[j].x - minv.x) / max_range;
      g_triangles[i].uv[j].v = (g_triangles[i].v[j].y - minv.y) / max_range;
    }
  }
}
