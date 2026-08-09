#include "scene.hpp"
#include "core/common.hpp"
#include "core/transform.hpp"
#include "ui.hpp"
#include <cstdio>

// Desenha o chão da cena: uma grade de linhas no plano Y=0 (XZ) que serve de
// referência espacial para ver a rotação e a posição do modelo. É desenhada
// sem iluminação, pois linhas não são afetadas por luz.
static void draw_grid_floor() {
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glColor3f(0.3f, 0.3f, 0.3f);
  float s = 0.5f;
  int n = 10;
  for (int i = -n; i < n; i++) {
    for (int j = -n; j < n; j++) {
      float x = i * s, z = j * s;
      glBegin(GL_LINE_LOOP);
      glVertex3f(x, 0.0f, z);
      glVertex3f(x + s, 0.0f, z);
      glVertex3f(x + s, 0.0f, z + s);
      glVertex3f(x, 0.0f, z + s);
      glEnd();
    }
  }
}

// Desenha uma lâmpada como uma esfera de arame (wireframe), indicando
// visualmente onde está cada fonte de luz da cena.
static void draw_lamp(const Vec3 &pos) {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glColor3f(0.9f, 0.9f, 0.5f);
  glutWireSphere(0.15f, 12, 8);

  glPopMatrix();
}

// Desenha os três eixos (X vermelho, Y verde, Z azul) transladados para um
// ponto dado. Usado na lâmpada selecionada para mostrar o espaço de arraste.
static void draw_axes_at(const Vec3 &pos) {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  float len = 0.5f;
  glLineWidth(3.0f);

  // Eixo X (vermelho) com uma seta na ponta.
  glColor3f(1, 0, 0);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(len, 0, 0);
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3f(len * 1.15f, 0, 0);
  glVertex3f(len * 0.85f, 0.05f, 0);
  glVertex3f(len * 0.85f, -0.05f, 0);
  glEnd();

  // Eixo Y (verde) com uma seta na ponta.
  glColor3f(0, 1, 0);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(0, len, 0);
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3f(0, len * 1.15f, 0);
  glVertex3f(0.05f, len * 0.85f, 0);
  glVertex3f(-0.05f, len * 0.85f, 0);
  glEnd();

  // Eixo Z (azul) com uma seta na ponta.
  glColor3f(0, 0, 1);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, len);
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3f(0, 0, len * 1.15f);
  glVertex3f(0.05f, 0, len * 0.85f);
  glVertex3f(-0.05f, 0, len * 0.85f);
  glEnd();

  // Rótulos das extremidades dos eixos.
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
}

// Linha do eixo em transformação: desenhada somente enquanto uma tecla de eixo
// (X/x, Y/y ou Z/z) está pressionada e alterando o modelo. Uma linha longa e
// fina ao longo do eixo ativo, na cor do eixo (X vermelho, Y verde, Z azul),
// partindo da posição atual do modelo e alinhada com os eixos globais (sem a
// rotação do modelo).
static void draw_active_axis_line() {
  if (g_active_axis < 0)
    return;

  glPushMatrix();
  glTranslatef(g_translate_x, g_translate_y, g_translate_z);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  float len = 10.0f;
  glLineWidth(2.0f);
  if (g_active_axis == 0)
    glColor3f(1, 0, 0);
  else if (g_active_axis == 1)
    glColor3f(0, 1, 0);
  else
    glColor3f(0, 0, 1);

  glBegin(GL_LINES);
  if (g_active_axis == 0) {
    glVertex3f(-len, 0, 0);
    glVertex3f(len, 0, 0);
  } else if (g_active_axis == 1) {
    glVertex3f(0, -len, 0);
    glVertex3f(0, len, 0);
  } else {
    glVertex3f(0, 0, -len);
    glVertex3f(0, 0, len);
  }
  glEnd();

  glPopMatrix();
}

// Indicador de orientação no canto superior direito da tela: mostra os eixos
// do mundo sempre alinhados com a rotação atual da câmera, para que o usuário
// saiba em qual direção está olhando. Desenha em um viewport pequeno e depois
// restaura o viewport original.
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

  // Câmera pequena com a mesma rotação da câmera principal.
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPerspective(45.0, 1.0, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0, 0, -3.0f);
  glRotatef(g_rot_x, 1, 0, 0);
  glRotatef(g_rot_y, 0, 1, 0);

  float len = 0.8f;
  glLineWidth(2.5f);

  // Eixo X (vermelho).
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

  // Eixo Y (verde).
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

  // Eixo Z (azul).
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

  // Rótulos dos eixos.
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

  // Desenha um retângulo em torno do indicador usando projeção ortográfica.
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

// Ponto de uma curva de Bézier cúbica, avaliado em t em [0,1] a partir dos
// quatro pontos de controle (b0, b1, b2, b3).
static Vec3 bezier_point(const Vec3 &b0, const Vec3 &b1, const Vec3 &b2,
                         const Vec3 &b3, float t) {
  float u = 1.0f - t;
  float u2 = u * u;
  float u3 = u2 * u;
  float t2 = t * t;
  float t3 = t2 * t;
  return b0 * u3 + b1 * (3.0f * u2 * t) + b2 * (3.0f * u * t2) + b3 * t3;
}

// Avalia a posição da câmera na curva da vitrine para o instante t em [0,1].
// Os 8 pontos de controle formam um anel ao redor do modelo; cada par de
// pontos consecutivos vira um segmento de Bézier cúbico, com as tangentes
// estimadas a partir dos pontos vizinhos (conversão de Hermite para Bézier).
// O resultado é uma curva fechada com continuidade C1 ao longo do trajeto.
static Vec3 eval_showcase_spline(float t) {
  static const Vec3 pts[] = {
      Vec3(3.5f, 0.8f, 0.0f),   Vec3(2.5f, 1.5f, 2.5f),
      Vec3(0.0f, 1.8f, 3.5f),   Vec3(-2.5f, 1.2f, 2.5f),
      Vec3(-3.5f, 0.5f, 0.0f),  Vec3(-2.5f, -0.2f, -2.5f),
      Vec3(0.0f, -0.5f, -3.5f), Vec3(2.5f, 0.2f, -2.5f),
  };
  const int n = 8;
  float seg = t * n;
  int idx = (int)seg;
  float frac = seg - idx;
  idx = idx % n;
  const Vec3 &p0 = pts[idx];
  const Vec3 &p1 = pts[(idx + 1) % n];
  // Tangentes nos pontos p0 e p1 (estimadas como Catmull-Rom) e conversão
  // Hermite -> Bézier: cada tangente vira um ponto de controle a 1/3 do trecho.
  Vec3 tangent0 = (pts[(idx + 1) % n] - pts[(idx - 1 + n) % n]) * 0.5f;
  Vec3 tangent1 = (pts[(idx + 2) % n] - pts[idx]) * 0.5f;
  Vec3 b0 = p0;
  Vec3 b1 = p0 + tangent0 * (1.0f / 3.0f);
  Vec3 b2 = p1 - tangent1 * (1.0f / 3.0f);
  Vec3 b3 = p1;
  return bezier_point(b0, b1, b2, b3, frac);
}

// Função principal de renderização (callback display). Ordem de desenho:
// limpa o buffer, configura a câmera, desenha a cena (chão, lâmpadas, modelo
// iluminado e texturizado), a curva da vitrine, o indicador de eixos e a
// interface; por fim troca os buffers (double buffering).
void render_scene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Projeção em perspectiva com a proporção da janela.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (double)g_win_w / (double)g_win_h, 0.1, 100.0);

  // Câmera: no modo vitrine, segue a curva spline olhando para a origem;
  // caso contrário, usa o zoom e a rotação controlados pelo mouse.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  if (g_showcase_active) {
    float t = g_showcase_time / g_showcase_duration;
    Vec3 pos = eval_showcase_spline(t);
    gluLookAt(pos.x, pos.y, pos.z, 0, 0, 0, 0, 1, 0);
  } else {
    glTranslatef(0, 0, -5.0f * (2.0f - g_zoom));
    glRotatef(g_rot_x, 1, 0, 0);
    glRotatef(g_rot_y, 0, 1, 0);
  }

  draw_grid_floor();

  // Desenha todas as lâmpadas da cena.
  for (size_t i = 0; i < g_lamp_positions.size(); i++)
    draw_lamp(g_lamp_positions[i]);

  // Mostra os eixos sobre a lâmpada selecionada.
  if (g_selected_lamp >= 0)
    draw_axes_at(g_lamp_positions[g_selected_lamp]);

  // Transformações do modelo (translação, rotação e escala não uniforme) via
  // matriz de transformação; GL_NORMALIZE (habilitado no init) mantém as
  // normais unitárias para a iluminação.
  glPushMatrix();
  apply_model_transform();

  if (g_model_loaded) {
    // Habilita iluminação com duas luzes posicionadas nas lâmpadas da cena.
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    GLfloat light0_pos[] = {g_lamp_positions[0].x, g_lamp_positions[0].y,
                            g_lamp_positions[0].z, 1.0f};
    GLfloat light1_pos[] = {g_lamp_positions[1].x, g_lamp_positions[1].y,
                            g_lamp_positions[1].z, 1.0f};
    GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat dim[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, dim);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);

    // Material: branco quando há textura (a cor vem da imagem) ou azulado
    // quando não há, com brilho especular leve.
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

    // Desenha cada triângulo da malha com sua normal e, quando há textura,
    // suas coordenadas UV.
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
    // Sem modelo, mostra um bule de teste para não deixar a cena vazia.
    glColor3f(0.4f, 0.4f, 0.6f);
    glutWireTeapot(1.0);
  }

  glPopMatrix();

  // Linha do eixo em transformação (visível apenas enquanto a tecla de eixo
  // estiver pressionada).
  draw_active_axis_line();

  // No modo vitrine, desenha a curva por onde a câmera está percorrendo.
  if (g_showcase_active) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.2f, 0.4f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 100; i++) {
      float t = i / 100.0f;
      Vec3 p = eval_showcase_spline(t);
      glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
    glLineWidth(1.0f);
  }

  draw_axes_indicator();

  // Interface em tela: botões e linha de informações.
  draw_button(20, 50, 120, 35, "Importar", g_import_hover);
  draw_button(150, 50, 120, 35, "Exportar", g_export_hover);
  draw_button(280, 50, 120, 35, "Textura", g_texture_hover);
  draw_button(410, 50, 120, 35, g_showcase_active ? "Showcase:ON" : "Showcase",
              g_showcase_hover);
  draw_info_text();

  glutSwapBuffers();
}
