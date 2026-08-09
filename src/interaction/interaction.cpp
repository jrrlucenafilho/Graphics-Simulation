#include "interaction.hpp"
#include <cmath>

// Seleciona a lâmpada mais próxima do cursor. Cada lâmpada é projetada da
// posição 3D para a tela com gluProject; a mais próxima dentro de um raio de
// tolerância (best_dist) é retornada. Lâmpadas atrás da câmera (sz fora de
// [0,1]) são ignoradas.
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
    // Projeta a posição 3D da lâmpada em coordenadas de tela.
    gluProject(g_lamp_positions[i].x, g_lamp_positions[i].y,
               g_lamp_positions[i].z, mv, proj, viewport, &sx, &sy, &sz);
    if (sz < 0.0 || sz >= 1.0)
      continue;
    // Distância euclidiana na tela (lembrando que a origem da tela em Y é
    // invertida em relação ao OpenGL).
    float dx = (float)mx - (float)sx;
    float dy = (float)(viewport[3] - my) - (float)sy;
    float d = std::sqrt(dx * dx + dy * dy);
    if (d < best_dist) {
      best_dist = d;
      best_idx = (int)i;
    }
  }
  return best_idx;
}

// Extrai a direção de visão da matriz modelview. A terceira coluna da matriz
// contém a direção Z da câmera; negada, aponta de volta ao mundo. Isso é usado
// para arrastar as lâmpadas no plano perpendicular à câmera.
Vec3 get_view_direction() {
  GLdouble mv[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, mv);
  Vec3 d(-(float)mv[2], -(float)mv[6], -(float)mv[10]);
  d.normalize();
  return d;
}

// Converte o mouse em um ponto 3D sobre um plano arbitrário: dispara um raio
// da câmera pelo pixel (gluUnProject nos planos near e far) e intersecta esse
// raio com o plano dado (equação paramétrica do plano via produto escalar).
// O resultado permite arrastar objetos mantendo-os sobre o plano do olhar.
Vec3 mouse_to_3d_plane(int mx, int my, const Vec3 &plane_normal,
                       const Vec3 &plane_pt) {
  GLint viewport[4];
  GLdouble mv[16], proj[16];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, mv);
  glGetDoublev(GL_PROJECTION_MATRIX, proj);

  // Desprojeta o pixel nos planos de recorte próximo (near) e distante (far);
  // o segmento entre esses dois pontos define o raio de visão.
  double nx, ny, nz, fx, fy, fz;
  gluUnProject(mx, viewport[3] - my, 0.0, mv, proj, viewport, &nx, &ny, &nz);
  gluUnProject(mx, viewport[3] - my, 1.0, mv, proj, viewport, &fx, &fy, &fz);

  // gluUnProject devolve doubles; os valores são convertidos para float ao
  // montar os vetores de trabalho da interseção.
  Vec3 orig((float)nx, (float)ny, (float)nz);
  Vec3 dir((float)(fx - nx), (float)(fy - ny), (float)(fz - nz));
  dir.normalize();

  // Interseção raio-plano: t = (pt - orig) . normal / (dir . normal).
  float denom = dir.dot(plane_normal);
  if (std::abs(denom) < 1e-6f)
    return plane_pt;
  float d = (plane_pt - orig).dot(plane_normal) / denom;
  return orig + dir * d;
}
