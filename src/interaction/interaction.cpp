#include "interaction.h"
#include <cmath>

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
