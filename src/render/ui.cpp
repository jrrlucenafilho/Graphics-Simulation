#include "ui.h"
#include <cstdio>
#include <cstring>

void draw_button(int x, int y, int w, int h, const char *label, bool hover) {
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

void draw_info_text() {
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
    snprintf(buf, sizeof(buf),
             "Triangulos: %zu | Arraste: rotacao | Scroll: zoom | P: pintar",
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

void draw_palette() {
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, g_win_w, g_win_h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  int ss = 50, sp = 20;
  int total_w = 3 * ss + 2 * sp;
  int start_x = (g_win_w - total_w) / 2;
  int y = 100;

  glColor3f(0.1f, 0.1f, 0.2f);
  glBegin(GL_QUADS);
  glVertex2i(start_x - 10, y - 5);
  glVertex2i(start_x + total_w + 10, y - 5);
  glVertex2i(start_x + total_w + 10, y + 20 + ss + 5);
  glVertex2i(start_x - 10, y + 20 + ss + 5);
  glEnd();

  glColor3f(0.5f, 0.5f, 0.5f);
  glBegin(GL_LINE_LOOP);
  glVertex2i(start_x - 10, y - 5);
  glVertex2i(start_x + total_w + 10, y - 5);
  glVertex2i(start_x + total_w + 10, y + 20 + ss + 5);
  glVertex2i(start_x - 10, y + 20 + ss + 5);
  glEnd();

  glColor3f(1, 1, 1);
  glWindowPos2i(start_x, g_win_h - y);
  const char *label = "Pintar:";
  for (const char *c = label; *c; c++)
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

  glColor3f(1, 0, 0);
  glBegin(GL_QUADS);
  glVertex2i(start_x, y + 15);
  glVertex2i(start_x + ss, y + 15);
  glVertex2i(start_x + ss, y + 15 + ss);
  glVertex2i(start_x, y + 15 + ss);
  glEnd();

  glColor3f(0, 1, 0);
  glBegin(GL_QUADS);
  glVertex2i(start_x + ss + sp, y + 15);
  glVertex2i(start_x + 2 * ss + sp, y + 15);
  glVertex2i(start_x + 2 * ss + sp, y + 15 + ss);
  glVertex2i(start_x + ss + sp, y + 15 + ss);
  glEnd();

  glColor3f(0, 0, 1);
  glBegin(GL_QUADS);
  glVertex2i(start_x + 2 * ss + 2 * sp, y + 15);
  glVertex2i(start_x + 3 * ss + 2 * sp, y + 15);
  glVertex2i(start_x + 3 * ss + 2 * sp, y + 15 + ss);
  glVertex2i(start_x + 2 * ss + 2 * sp, y + 15 + ss);
  glEnd();

  int sel_idx = (g_selected_color.x == 1.0f && g_selected_color.y == 0.0f) ? 0
                : (g_selected_color.y == 1.0f)                             ? 1
                                                                           : 2;
  int hx = start_x + sel_idx * (ss + sp);
  glColor3f(1, 1, 0);
  glBegin(GL_LINE_LOOP);
  glVertex2i(hx - 2, y + 13);
  glVertex2i(hx + ss + 2, y + 13);
  glVertex2i(hx + ss + 2, y + 15 + ss + 2);
  glVertex2i(hx - 2, y + 15 + ss + 2);
  glEnd();

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
}
