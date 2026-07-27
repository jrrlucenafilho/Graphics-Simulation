#ifndef COMMON_H
#define COMMON_H

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <cmath>
#include <string>
#include <vector>

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
  Vec3 operator-() const { return Vec3(-x, -y, -z); }
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
  Vec3 color;
};

extern std::vector<Triangle> g_triangles;
extern bool g_model_loaded;
extern float g_rot_x, g_rot_y;

extern int g_last_mx, g_last_my;
extern bool g_dragging;
extern float g_zoom;
extern int g_win_w, g_win_h;
extern bool g_import_hover;
extern bool g_export_hover;
extern bool g_texture_hover;
extern GLuint g_texture_id;
extern bool g_texture_loaded;
extern bool g_paint_mode;
extern bool g_show_palette;
extern Vec3 g_selected_color;
extern bool g_paint_hover;
extern bool g_has_painted_faces;
extern bool g_showcase_hover;
extern bool g_showcase_active;
extern float g_showcase_time;
extern float g_showcase_duration;
extern std::vector<Vec3> g_lamp_positions;
extern int g_selected_lamp;

#endif
