#include "stl_io.h"
#include <algorithm>
#include <cstdio>
#include <fstream>

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
      if (trim.substr(0, 12) == "facet normal")
        sscanf(trim.c_str(), "facet normal %f %f %f", &tri.normal.x,
               &tri.normal.y, &tri.normal.z);
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
          sscanf(t.c_str(), "vertex %f %f %f", &tri.v[vi].x, &tri.v[vi].y,
                 &tri.v[vi].z);
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
  f.read((char *)&count, 4);
  for (unsigned int i = 0; i < count; i++) {
    Triangle tri;
    f.read((char *)&tri.normal, sizeof(float) * 3);
    f.read((char *)&tri.v[0], sizeof(float) * 3);
    f.read((char *)&tri.v[1], sizeof(float) * 3);
    f.read((char *)&tri.v[2], sizeof(float) * 3);
    unsigned short attr;
    f.read((char *)&attr, 2);
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
