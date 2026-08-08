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
             "Triangulos: %zu | Arraste: rotacao | Scroll: zoom | "
             "X/x Y/y Z/z: esticar (%.2f, %.2f, %.2f)",
             g_triangles.size(), g_scale_x, g_scale_y, g_scale_z);
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
