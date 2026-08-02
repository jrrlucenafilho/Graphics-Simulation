#include "scene.h"
#include "core/common.h"
#include "ui.h"
#include <cstdio>

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

static void draw_lamp(const Vec3 &pos) {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glColor3f(0.9f, 0.9f, 0.5f);
  glutWireSphere(0.15f, 12, 8);

  glPopMatrix();
}

static void draw_axes_at(const Vec3 &pos) {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  float len = 0.5f;
  glLineWidth(3.0f);

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

static Vec3 catmull_rom(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2,
                         const Vec3 &p3, float t) {
  float t2 = t * t;
  float t3 = t2 * t;
  return (p1 * 2.0f +
          (p2 - p0) * t +
          (p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * t2 +
          (-p0 + p1 * 3.0f - p2 * 3.0f + p3) * t3) * 0.5f;
}

static Vec3 eval_showcase_spline(float t) {
  static const Vec3 pts[] = {
    Vec3(3.5f, 0.8f, 0.0f),  Vec3(2.5f, 1.5f, 2.5f),
    Vec3(0.0f, 1.8f, 3.5f),  Vec3(-2.5f, 1.2f, 2.5f),
    Vec3(-3.5f, 0.5f, 0.0f), Vec3(-2.5f, -0.2f, -2.5f),
    Vec3(0.0f, -0.5f, -3.5f),Vec3(2.5f, 0.2f, -2.5f),
  };
  const int n = 8;
  float seg = t * n;
  int idx = (int)seg;
  float frac = seg - idx;
  idx = idx % n;
  const Vec3 &p0 = pts[(idx - 1 + n) % n];
  const Vec3 &p1 = pts[idx];
  const Vec3 &p2 = pts[(idx + 1) % n];
  const Vec3 &p3 = pts[(idx + 2) % n];
  return catmull_rom(p0, p1, p2, p3, frac);
}

void render_scene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (double)g_win_w / (double)g_win_h, 0.1, 100.0);

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

  for (size_t i = 0; i < g_lamp_positions.size(); i++)
    draw_lamp(g_lamp_positions[i]);

  if (g_selected_lamp >= 0)
    draw_axes_at(g_lamp_positions[g_selected_lamp]);

  // Escala não uniforme via matriz de transformação; GL_NORMALIZE (habilitado
  // no init) mantém as normais unitárias para a iluminação.
  glPushMatrix();
  glScalef(g_scale_x, g_scale_y, g_scale_z);

  if (g_model_loaded) {
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

    if (g_has_painted_faces) {
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    }

    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < g_triangles.size(); i++) {
      if (g_has_painted_faces)
        glColor3f(g_triangles[i].color.x, g_triangles[i].color.y,
                  g_triangles[i].color.z);
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
    if (g_has_painted_faces)
      glDisable(GL_COLOR_MATERIAL);
    if (g_texture_loaded)
      glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
  } else {
    glColor3f(0.4f, 0.4f, 0.6f);
    glutWireTeapot(1.0);
  }

  glPopMatrix();

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

  if (g_show_palette)
    draw_palette();

  draw_axes_indicator();

  draw_button(20, 50, 120, 35, "Importar", g_import_hover);
  draw_button(150, 50, 120, 35, "Exportar", g_export_hover);
  draw_button(280, 50, 120, 35, "Textura", g_texture_hover);
  draw_button(410, 50, 120, 35, g_paint_mode ? "Pintar:ON" : "Pintar",
              g_paint_hover);
  draw_button(540, 50, 120, 35, g_showcase_active ? "Showcase:ON" : "Showcase",
              g_showcase_hover);
  draw_info_text();

  glutSwapBuffers();
}
