#include "ui.hpp"
#include "../core/transform.hpp"
#include <cstdio>
#include <cstring>

// Desenha um botão 2D. A projeção é trocada temporariamente para ortográfica
// (em pixels da janela) para que o desenho seja independente da câmera 3D;
// depois a projeção original é restaurada. Composto por: fundo (quad), borda
// (line loop) e rótulo (texto com glutBitmapCharacter).
void draw_button(int x, int y, int w, int h, const char *label, bool hover) {
  // Projeção ortográfica cobrindo toda a janela, com origem no canto
  // superior esquerdo (g_win_h no topo).
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, g_win_w, g_win_h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Fundo do botão; cor mais clara quando o cursor está sobre ele (hover).
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

  // Borda branca ao redor do botão.
  glColor3f(1, 1, 1);
  glBegin(GL_LINE_LOOP);
  glVertex2i(x, y);
  glVertex2i(x + w, y);
  glVertex2i(x + w, y + h);
  glVertex2i(x, y + h);
  glEnd();

  // Rótulo centralizado aproximadamente (4px por caractere na fonte 8x13).
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

// Desenha a linha de informações no rodapé da janela: mostra o número de
// triângulos do modelo carregado, as instruções de uso e os valores atuais de
// escala de cada eixo. Também usa projeção ortográfica para ficar sempre
// legível, independente da câmera.
void draw_info_text() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, g_win_w, g_win_h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(0.8f, 0.8f, 0.8f);
  char line1[256];
  char line2[256];
  const char *line3 =
      "1: mover | 2: girar | 3: esticar | X/Y/Z: eixo do modo (maiscula +, "
      "minuscula -) | R: reset";

  if (g_model_loaded) {
    snprintf(line1, sizeof(line1), "Triangulos: %zu | Modo: %s",
             g_triangles.size(), transform_mode_name());
    snprintf(line2, sizeof(line2),
             "T=(%.2f, %.2f, %.2f) | R=(%.0f, %.0f, %.0f) | "
             "S=(%.2f, %.2f, %.2f)",
             g_translate_x, g_translate_y, g_translate_z, g_model_rot_x,
             g_model_rot_y, g_model_rot_z, g_scale_x, g_scale_y, g_scale_z);
  } else {
    snprintf(line1, sizeof(line1),
             "Nenhum modelo carregado. Clique em Importar. | Modo: %s",
             transform_mode_name());
    snprintf(line2, sizeof(line2),
             "O bule de teste tambem responde as transformacoes.");
  }

  const char *lines[] = {line1, line2, line3};
  const int y_positions[] = {50, 34, 18};
  for (int i = 0; i < 3; i++) {
    glWindowPos2i(10, y_positions[i]);
    for (const char *c = lines[i]; *c; c++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}
