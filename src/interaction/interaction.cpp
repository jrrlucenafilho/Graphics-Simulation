#include "interaction.h"
#include <cmath>

static bool ray_triangle_intersect(const Vec3 &orig, const Vec3 &dir,
                                   const Triangle &tri, float &t) {
  const float EPS = 1e-6f;
  Vec3 e1 = tri.v[1] - tri.v[0];
  Vec3 e2 = tri.v[2] - tri.v[0];
  Vec3 pvec = dir.cross(e2);
  float det = e1.dot(pvec);
  if (std::abs(det) < EPS)
    return false;
  float inv_det = 1.0f / det;
  Vec3 tvec = orig - tri.v[0];
  float u = tvec.dot(pvec) * inv_det;
  if (u < 0 || u > 1)
    return false;
  Vec3 qvec = tvec.cross(e1);
  float v = dir.dot(qvec) * inv_det;
  if (v < 0 || u + v > 1)
    return false;
  t = e2.dot(qvec) * inv_det;
  return t > 0;
}

int pick_triangle(int mx, int my) {
  GLint viewport[4];
  GLdouble mv[16], proj[16];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, mv);
  glGetDoublev(GL_PROJECTION_MATRIX, proj);

  double nx, ny, nz, fx, fy, fz;
  gluUnProject(mx, viewport[3] - my, 0.0, mv, proj, viewport, &nx, &ny, &nz);
  gluUnProject(mx, viewport[3] - my, 1.0, mv, proj, viewport, &fx, &fy, &fz);

  // O modelo é desenhado com glScalef, mas os triângulos ficam armazenados
  // sem escala; leva o raio para o espaço do modelo com a escala inversa.
  Vec3 orig(nx / g_scale_x, ny / g_scale_y, nz / g_scale_z);
  Vec3 dir((fx - nx) / g_scale_x, (fy - ny) / g_scale_y,
           (fz - nz) / g_scale_z);
  dir.normalize();

  float closest = 1e10f;
  int hit = -1;
  for (size_t i = 0; i < g_triangles.size(); i++) {
    float t;
    if (ray_triangle_intersect(orig, dir, g_triangles[i], t)) {
      if (t < closest) {
        closest = t;
        hit = (int)i;
      }
    }
  }
  return hit;
}

int pick_lamp(int mx, int my) {
  GLint viewport[4];
  GLdouble mv[16], proj[16];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, mv);
  glGetDoublev(GL_PROJECTION_MATRIX, proj);

  double sx, sy, sz;
  float best_dist = 25.0f;
  int best_idx = -1;

  for (size_t i = 0; i < g_lamp_positions.size(); i++) {
    gluProject(g_lamp_positions[i].x, g_lamp_positions[i].y,
               g_lamp_positions[i].z, mv, proj, viewport, &sx, &sy, &sz);
    if (sz < 0.0 || sz >= 1.0)
      continue;
    float d = sqrtf((mx - sx) * (mx - sx) +
                    ((viewport[3] - my) - sy) * ((viewport[3] - my) - sy));
    if (d < best_dist) {
      best_dist = d;
      best_idx = (int)i;
    }
  }
  return best_idx;
}

Vec3 get_view_direction() {
  GLdouble mv[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, mv);
  Vec3 d(-mv[2], -mv[6], -mv[10]);
  d.normalize();
  return d;
}

Vec3 mouse_to_3d_plane(int mx, int my, const Vec3 &plane_normal,
                        const Vec3 &plane_pt) {
  GLint viewport[4];
  GLdouble mv[16], proj[16];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, mv);
  glGetDoublev(GL_PROJECTION_MATRIX, proj);

  double nx, ny, nz, fx, fy, fz;
  gluUnProject(mx, viewport[3] - my, 0.0, mv, proj, viewport, &nx, &ny, &nz);
  gluUnProject(mx, viewport[3] - my, 1.0, mv, proj, viewport, &fx, &fy, &fz);

  Vec3 orig(nx, ny, nz);
  Vec3 dir(fx - nx, fy - ny, fz - nz);
  dir.normalize();

  float denom = dir.dot(plane_normal);
  if (fabs(denom) < 1e-6f)
    return plane_pt;
  float d = (plane_pt - orig).dot(plane_normal) / denom;
  return orig + dir * d;
}
