#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

struct Vec3 {
  float x, y, z;
  Vec3() : x(0), y(0), z(0) {}
  Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
  Vec3 operator-(const Vec3 &o) const {
    return Vec3(x - o.x, y - o.y, z - o.z);
  }
  Vec3 operator+(const Vec3 &o) const {
    return Vec3(x + o.x, y + o.y, z + o.z);
  }
  Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
  float dot(const Vec3 &o) const { return x * o.x + y * o.y + z * o.z; }
  Vec3 cross(const Vec3 &o) const {
    return Vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
  }
  float length() const { return sqrtf(x * x + y * y + z * z); }
  void normalize() {
    float l = length();
    if (l > 1e-8f) {
      x /= l;
      y /= l;
      z /= l;
    }
  }
};

struct Vec2 {
  float u, v;
  Vec2() : u(0), v(0) {}
  Vec2(float u, float v) : u(u), v(v) {}
};

struct Triangle {
  Vec3 normal, v[3];
  Vec2 uv[3];
};

static std::vector<Triangle> g_triangles;
static bool g_model_loaded = false;
static float g_rot_x = 0.0f, g_rot_y = 0.0f;
static float g_auto_rot = 0.0f;
static bool g_auto_rot_enabled = false;
static int g_last_mx = 0, g_last_my = 0;
static bool g_dragging = false;
static float g_zoom = 1.0f;
static int g_win_w = WINDOW_WIDTH, g_win_h = WINDOW_HEIGHT;

static bool g_import_hover = false;
static bool g_export_hover = false;
static bool g_rotate_hover = false;
static bool g_texture_hover = false;

static GLuint g_texture_id = 0;
static bool g_texture_loaded = false;

// ---- STL loader ----

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

static bool load_stl(const std::string &path) {
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

// ---- STL exporter ----

static bool export_stl_ascii(const std::string &path) {
  std::ofstream f(path.c_str());
  if (!f)
    return false;
  f << "solid exported\n";
  for (size_t i = 0; i < g_triangles.size(); i++) {
    f << "  facet normal " << g_triangles[i].normal.x << " "
      << g_triangles[i].normal.y << " " << g_triangles[i].normal.z << "\n";
    f << "    outer loop\n";
    for (int j = 0; j < 3; j++)
      f << "      vertex " << g_triangles[i].v[j].x << " "
        << g_triangles[i].v[j].y << " " << g_triangles[i].v[j].z << "\n";
    f << "    endloop\n";
    f << "  endfacet\n";
  }
  f << "endsolid exported\n";
  return true;
}

// ---- bounding box & centering ----

static void center_model() {
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

// ---- file dialog helpers ----

static std::string open_file_dialog() {
  std::string cmd = "zenity --file-selection --title=\"Selecionar modelo STL\" "
                    "--file-filter=\"STL files (*.stl) | *.stl\" 2>/dev/null";
  FILE *fp = popen(cmd.c_str(), "r");
  if (!fp)
    return "";
  char buf[4096] = {0};
  if (fgets(buf, sizeof(buf) - 1, fp)) {
    pclose(fp);
    std::string s(buf);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
      s.pop_back();
    return s;
  }
  pclose(fp);
  return "";
}

static std::string save_file_dialog() {
  std::string cmd =
      "zenity --file-selection --save --title=\"Exportar modelo STL\" "
      "--file-filter=\"STL files (*.stl) | *.stl\" "
      "--filename=\"modelo_exportado.stl\" 2>/dev/null";
  FILE *fp = popen(cmd.c_str(), "r");
  if (!fp)
    return "";
  char buf[4096] = {0};
  if (fgets(buf, sizeof(buf) - 1, fp)) {
    pclose(fp);
    std::string s(buf);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
      s.pop_back();
    if (s.find('.') == std::string::npos)
      s += ".stl";
    return s;
  }
  pclose(fp);
  return "";
}

// ---- texture helpers ----

static std::string open_texture_dialog() {
  std::string cmd =
      "zenity --file-selection --title=\"Selecionar textura\" "
      "--file-filter=\"Imagens (*.jpg *.jpeg *.png) | *.jpg *.jpeg *.png\" "
      "2>/dev/null";
  FILE *fp = popen(cmd.c_str(), "r");
  if (!fp)
    return "";
  char buf[4096] = {0};
  if (fgets(buf, sizeof(buf) - 1, fp)) {
    pclose(fp);
    std::string s(buf);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
      s.pop_back();
    return s;
  }
  pclose(fp);
  return "";
}

static bool load_texture(const std::string &path) {
  int w, h, channels;
  unsigned char *data = stbi_load(path.c_str(), &w, &h, &channels, 3);
  if (!data) {
    printf("Erro ao carregar textura: %s\n", path.c_str());
    return false;
  }

  if (g_texture_loaded)
    glDeleteTextures(1, &g_texture_id);

  glGenTextures(1, &g_texture_id);
  glBindTexture(GL_TEXTURE_2D, g_texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,
               data);

  stbi_image_free(data);
  g_texture_loaded = true;
  printf("Textura carregada: %s (%dx%d)\n", path.c_str(), w, h);
  return true;
}

static void generate_uv_coords() {
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

// ---- OpenGL drawing ----

static void draw_button(int x, int y, int w, int h, const char *label,
                        bool hover) {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, g_win_w, g_win_h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if (hover) {
    glColor3f(0.3f, 0.6f, 1.0f);
  } else {
    glColor3f(0.2f, 0.4f, 0.8f);
  }
  glBegin(GL_QUADS);
  glVertex2i(x, y);
  glVertex2i(x + w, y);
  glVertex2i(x + w, y + h);
  glVertex2i(x, y + h);
  glEnd();

  glColor3f(1, 1, 1);
  glBegin(GL_LINE_LOOP);
  glVertex2i(x, y);
  glVertex2i(x + w, y);
  glVertex2i(x + w, y + h);
  glVertex2i(x, y + h);
  glEnd();

  glColor3f(1, 1, 1);
  int len = (int)strlen(label);
  glWindowPos2i(x + w / 2 - len * 4, g_win_h - (y + h / 2 + 4));
  for (const char *c = label; *c; c++)
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

static void draw_info_text() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, g_win_w, g_win_h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(0.8f, 0.8f, 0.8f);
  char buf[256];
  if (g_model_loaded) {
    snprintf(
        buf, sizeof(buf),
        "Triangulos: %zu | Arraste: rotacao | Scroll: zoom | R: rotacao auto",
        g_triangles.size());
  } else {
    snprintf(buf, sizeof(buf), "Nenhum modelo carregado. Clique em Importar.");
  }
  glWindowPos2i(10, g_win_h - 28);
  for (const char *c = buf; *c; c++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

static void draw_axes_indicator() {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  int size = 90;
  int margin = 10;
  int vx = g_win_w - margin - size;
  int vy = g_win_h - margin - size;
  glViewport(vx, vy, size, size);

  glClear(GL_DEPTH_BUFFER_BIT);
  glDisable(GL_LIGHTING);

  // 3D axes with same camera rotation as the main scene
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPerspective(45.0, 1.0, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0, 0, -3.0f);
  glRotatef(g_rot_x, 1, 0, 0);
  if (g_auto_rot_enabled)
    glRotatef(g_auto_rot, 0, 1, 0);
  glRotatef(g_rot_y, 0, 1, 0);

  float len = 0.8f;
  glLineWidth(2.5f);

  // X axis - Red
  glColor3f(1, 0, 0);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(len, 0, 0);
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3f(len * 1.15f, 0, 0);
  glVertex3f(len * 0.85f, 0.06f, 0);
  glVertex3f(len * 0.85f, -0.06f, 0);
  glEnd();

  // Y axis - Green
  glColor3f(0, 1, 0);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(0, len, 0);
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3f(0, len * 1.15f, 0);
  glVertex3f(0.06f, len * 0.85f, 0);
  glVertex3f(-0.06f, len * 0.85f, 0);
  glEnd();

  // Z axis - Blue
  glColor3f(0, 0, 1);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, len);
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3f(0, 0, len * 1.15f);
  glVertex3f(0.06f, 0, len * 0.85f);
  glVertex3f(-0.06f, 0, len * 0.85f);
  glEnd();

  // Labels
  glDisable(GL_DEPTH_TEST);
  glColor3f(1, 0, 0);
  glRasterPos3f(len * 1.3f, 0, 0);
  glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'X');
  glColor3f(0, 1, 0);
  glRasterPos3f(0, len * 1.3f, 0);
  glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Y');
  glColor3f(0, 0, 1);
  glRasterPos3f(0, 0, len * 1.3f);
  glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Z');
  glEnable(GL_DEPTH_TEST);

  glLineWidth(1.0f);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  // Border around the indicator
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, g_win_w, 0, g_win_h, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(0.5f, 0.5f, 0.5f);
  glBegin(GL_LINE_LOOP);
  glVertex2i(vx, vy);
  glVertex2i(vx + size, vy);
  glVertex2i(vx + size, vy + size);
  glVertex2i(vx, vy + size);
  glEnd();

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

static void render_scene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (double)g_win_w / (double)g_win_h, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, -5.0f * (2.0f - g_zoom));
  glRotatef(g_rot_x, 1, 0, 0);
  if (g_auto_rot_enabled)
    glRotatef(g_auto_rot, 0, 1, 0);
  glRotatef(g_rot_y, 0, 1, 0);

  if (g_model_loaded) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    GLfloat light0_pos[] = {5.0f, 5.0f, 5.0f, 1.0f};
    GLfloat light1_pos[] = {-5.0f, -3.0f, 2.0f, 1.0f};
    GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat dim[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, dim);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);

    if (g_texture_loaded) {
      GLfloat mat_diff_tex[] = {1.0f, 1.0f, 1.0f, 1.0f};
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diff_tex);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, g_texture_id);
    } else {
      GLfloat mat_diff[] = {0.7f, 0.7f, 0.9f, 1.0f};
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diff);
    }
    GLfloat mat_spec[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat mat_shin[] = {30.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shin);

    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < g_triangles.size(); i++) {
      glNormal3f(g_triangles[i].normal.x, g_triangles[i].normal.y,
                 g_triangles[i].normal.z);
      for (int j = 0; j < 3; j++) {
        if (g_texture_loaded)
          glTexCoord2f(g_triangles[i].uv[j].u, g_triangles[i].uv[j].v);
        glVertex3f(g_triangles[i].v[j].x, g_triangles[i].v[j].y,
                   g_triangles[i].v[j].z);
      }
    }
    glEnd();
    if (g_texture_loaded)
      glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
  } else {
    glColor3f(0.4f, 0.4f, 0.6f);
    glutWireTeapot(1.0);
  }

  draw_axes_indicator();

  draw_button(20, 50, 120, 35, "Importar", g_import_hover);
  draw_button(150, 50, 120, 35, "Exportar", g_export_hover);
  draw_button(280, 50, 120, 35,
              g_auto_rot_enabled ? "Rotacao:ON" : "Rotacao:OFF",
              g_rotate_hover);
  draw_button(410, 50, 120, 35, "Textura", g_texture_hover);
  draw_info_text();

  glutSwapBuffers();
}

// ---- callbacks ----

static void display() { render_scene(); }

static void reshape(int w, int h) {
  g_win_w = w;
  g_win_h = h;
  glViewport(0, 0, w, h);
}

static void idle() {
  if (g_auto_rot_enabled) {
    g_auto_rot += 0.3f;
    if (g_auto_rot > 360.0f)
      g_auto_rot -= 360.0f;
    glutPostRedisplay();
  }
}

static void mouse(int button, int state, int x, int y) {
  bool over_import = (x >= 20 && x <= 140 && y >= 50 && y <= 85);
  bool over_export = (x >= 150 && x <= 270 && y >= 50 && y <= 85);
  bool over_rotate = (x >= 280 && x <= 400 && y >= 50 && y <= 85);
  bool over_texture = (x >= 410 && x <= 530 && y >= 50 && y <= 85);

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    if (over_import) {
      std::string path = open_file_dialog();
      if (!path.empty()) {
        if (load_stl(path)) {
          center_model();
          generate_uv_coords();
          printf("Modelo carregado: %s (%zu triangulos)\n", path.c_str(),
                 g_triangles.size());
        } else {
          printf("Erro ao carregar: %s\n", path.c_str());
        }
      }
      return;
    }
    if (over_export) {
      if (!g_model_loaded) {
        printf("Nenhum modelo para exportar.\n");
        return;
      }
      std::string path = save_file_dialog();
      if (!path.empty()) {
        if (export_stl_ascii(path)) {
          printf("Modelo exportado: %s\n", path.c_str());
        } else {
          printf("Erro ao exportar.\n");
        }
      }
      return;
    }
    if (over_rotate) {
      g_auto_rot_enabled = !g_auto_rot_enabled;
      printf("Rotacao automatica: %s\n",
             g_auto_rot_enabled ? "LIGADA" : "DESLIGADA");
      glutPostRedisplay();
      return;
    }
    if (over_texture) {
      std::string path = open_texture_dialog();
      if (!path.empty()) {
        if (load_texture(path)) {
          if (!g_model_loaded) {
            printf("A textura sera exibida quando um modelo for carregado.\n");
          }
          glutPostRedisplay();
        }
      }
      return;
    }
    g_dragging = true;
    g_last_mx = x;
    g_last_my = y;
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    g_dragging = false;
  }

  if (button == 3) {
    g_zoom = std::min(g_zoom + 0.1f, 3.0f);
  }
  if (button == 4) {
    g_zoom = std::max(g_zoom - 0.1f, 0.3f);
  }
  glutPostRedisplay();
}

static void motion(int x, int y) {
  if (g_dragging) {
    int dx = x - g_last_mx;
    int dy = y - g_last_my;
    g_rot_y += dx * 0.5f;
    g_rot_x += dy * 0.5f;
    g_last_mx = x;
    g_last_my = y;
    glutPostRedisplay();
  }

  g_import_hover = (x >= 20 && x <= 140 && y >= 50 && y <= 85);
  g_export_hover = (x >= 150 && x <= 270 && y >= 50 && y <= 85);
  g_rotate_hover = (x >= 280 && x <= 400 && y >= 50 && y <= 85);
  g_texture_hover = (x >= 410 && x <= 530 && y >= 50 && y <= 85);
  glutPostRedisplay();
}

static void keyboard(unsigned char key, int, int) {
  switch (key) {
  case 'i':
  case 'I': {
    std::string path = open_file_dialog();
    if (!path.empty()) {
      if (load_stl(path)) {
        center_model();
        generate_uv_coords();
        printf("Modelo carregado: %s (%zu triangulos)\n", path.c_str(),
               g_triangles.size());
      } else {
        printf("Erro ao carregar: %s\n", path.c_str());
      }
    }
    break;
  }
  case 'e':
  case 'E': {
    if (!g_model_loaded) {
      printf("Nenhum modelo.\n");
      break;
    }
    std::string path = save_file_dialog();
    if (!path.empty()) {
      if (export_stl_ascii(path))
        printf("Exportado: %s\n", path.c_str());
      else
        printf("Erro ao exportar.\n");
    }
    break;
  }
  case 't':
  case 'T': {
    std::string path = open_texture_dialog();
    if (!path.empty()) {
      if (load_texture(path)) {
        if (!g_model_loaded) {
          printf("A textura sera exibida quando um modelo for carregado.\n");
        }
        glutPostRedisplay();
      }
    }
    break;
  }
  case 'r':
  case 'R':
    g_auto_rot_enabled = !g_auto_rot_enabled;
    printf("Rotacao automatica: %s\n",
           g_auto_rot_enabled ? "LIGADA" : "DESLIGADA");
    glutPostRedisplay();
    break;
  case 27:
    exit(0);
  }
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(g_win_w, g_win_h);
  glutCreateWindow("Visualizador 3D STL - Computacao Grafica");

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glClearColor(0.15f, 0.15f, 0.2f, 1.0f);

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(idle);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(motion);
  glutKeyboardFunc(keyboard);

  if (argc > 1) {
    if (load_stl(argv[1])) {
      center_model();
      generate_uv_coords();
      printf("Modelo carregado: %s (%zu triangulos)\n", argv[1],
             g_triangles.size());
    }
  }

  glutMainLoop();
  return 0;
}
