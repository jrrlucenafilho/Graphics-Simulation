#include "stl_io.h"
#include <algorithm>
#include <cstdio>
#include <fstream>

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

bool export_stl_ascii(const std::string &path) {
  std::ofstream f(path.c_str());
  if (!f)
    return false;
  // Aplica a escala atual (feita via matriz na renderização) aos vértices
  // exportados, para que o arquivo reflita o modelo como exibido na tela.
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
