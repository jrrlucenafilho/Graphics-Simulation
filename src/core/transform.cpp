#include "transform.hpp"
#include <cmath>
#include <cstdio>

float g_translate_x = 0.0f, g_translate_y = 0.0f, g_translate_z = 0.0f;
float g_model_rot_x = 0.0f, g_model_rot_y = 0.0f, g_model_rot_z = 0.0f;
float g_scale_x = 1.0f, g_scale_y = 1.0f, g_scale_z = 1.0f;
TransformMode g_transform_mode = TRANSFORM_TRANSLATE;

#define TRANSLATE_STEP 0.10f
#define ROTATE_STEP 5.0f
#define SCALE_STEP 1.10f
#define SCALE_MIN 0.10f
#define SCALE_MAX 10.0f

static float degrees_to_radians(float degrees)
{
  return degrees * 3.14159265358979323846f / 180.0f;
}

static Vec3 rotate_x(const Vec3 &v, float degrees)
{
  float a = degrees_to_radians(degrees);
  float c = cosf(a);
  float s = sinf(a);
  return Vec3(v.x, c * v.y - s * v.z, s * v.y + c * v.z);
}

static Vec3 rotate_y(const Vec3 &v, float degrees)
{
  float a = degrees_to_radians(degrees);
  float c = cosf(a);
  float s = sinf(a);
  return Vec3(c * v.x + s * v.z, v.y, -s * v.x + c * v.z);
}

static Vec3 rotate_z(const Vec3 &v, float degrees)
{
  float a = degrees_to_radians(degrees);
  float c = cosf(a);
  float s = sinf(a);
  return Vec3(c * v.x - s * v.y, s * v.x + c * v.y, v.z);
}

static float normalize_angle(float angle)
{
  while (angle >= 360.0f)
    angle -= 360.0f;
  while (angle <= -360.0f)
    angle += 360.0f;
  return angle;
}

static void print_transform_state()
{
  printf("Modo: %s | T=(%.2f, %.2f, %.2f) | R=(%.1f, %.1f, %.1f) | "
         "S=(%.2f, %.2f, %.2f)\n",
         transform_mode_name(), g_translate_x, g_translate_y, g_translate_z,
         g_model_rot_x, g_model_rot_y, g_model_rot_z, g_scale_x, g_scale_y,
         g_scale_z);
}

static void request_redraw()
{
  print_transform_state();
  glutPostRedisplay();
}

static void apply_scale(float &axis_scale, float factor)
{
  float next = axis_scale * factor;
  if (next < SCALE_MIN || next > SCALE_MAX)
    return;
  axis_scale = next;
  request_redraw();
}

const char *transform_mode_name()
{
  switch (g_transform_mode)
  {
  case TRANSFORM_TRANSLATE:
    return "TRANSLACAO";
  case TRANSFORM_ROTATE:
    return "ROTACAO";
  case TRANSFORM_SCALE:
    return "ESCALA";
  }
  return "DESCONHECIDO";
}

void reset_model_transform()
{
  g_translate_x = 0.0f;
  g_translate_y = 0.0f;
  g_translate_z = 0.0f;

  g_model_rot_x = 0.0f;
  g_model_rot_y = 0.0f;
  g_model_rot_z = 0.0f;

  g_scale_x = 1.0f;
  g_scale_y = 1.0f;
  g_scale_z = 1.0f;

  request_redraw();
}

bool handle_transform_key(unsigned char key)
{
  switch (key)
  {
  case '1':
    g_transform_mode = TRANSFORM_TRANSLATE;
    request_redraw();
    return true;
  case '2':
    g_transform_mode = TRANSFORM_ROTATE;
    request_redraw();
    return true;
  case '3':
    g_transform_mode = TRANSFORM_SCALE;
    request_redraw();
    return true;
  case 'r':
  case 'R':
    reset_model_transform();
    return true;

  // Atalhos de escala já existentes no projeto.
  case 'X':
    apply_scale(g_scale_x, SCALE_STEP);
    return true;
  case 'x':
    apply_scale(g_scale_x, 1.0f / SCALE_STEP);
    return true;
  case 'Y':
    apply_scale(g_scale_y, SCALE_STEP);
    return true;
  case 'y':
    apply_scale(g_scale_y, 1.0f / SCALE_STEP);
    return true;
  case 'Z':
    apply_scale(g_scale_z, SCALE_STEP);
    return true;
  case 'z':
    apply_scale(g_scale_z, 1.0f / SCALE_STEP);
    return true;
  }
  return false;
}

bool handle_transform_special_key(int key)
{
  float direction = 0.0f;
  int axis = -1; // 0 = X, 1 = Y, 2 = Z

  switch (key)
  {
  case GLUT_KEY_LEFT:
    axis = 0;
    direction = -1.0f;
    break;
  case GLUT_KEY_RIGHT:
    axis = 0;
    direction = 1.0f;
    break;
  case GLUT_KEY_DOWN:
    axis = 1;
    direction = -1.0f;
    break;
  case GLUT_KEY_UP:
    axis = 1;
    direction = 1.0f;
    break;
  case GLUT_KEY_PAGE_DOWN:
    axis = 2;
    direction = -1.0f;
    break;
  case GLUT_KEY_PAGE_UP:
    axis = 2;
    direction = 1.0f;
    break;
  default:
    return false;
  }

  if (g_transform_mode == TRANSFORM_TRANSLATE)
  {
    float delta = TRANSLATE_STEP * direction;
    if (axis == 0)
      g_translate_x += delta;
    else if (axis == 1)
      g_translate_y += delta;
    else
      g_translate_z += delta;
  }
  else if (g_transform_mode == TRANSFORM_ROTATE)
  {
    float delta = ROTATE_STEP * direction;
    if (axis == 0)
      g_model_rot_x = normalize_angle(g_model_rot_x + delta);
    else if (axis == 1)
      g_model_rot_y = normalize_angle(g_model_rot_y + delta);
    else
      g_model_rot_z = normalize_angle(g_model_rot_z + delta);
  }
  else
  {
    float factor = direction > 0.0f ? SCALE_STEP : 1.0f / SCALE_STEP;
    if (axis == 0)
      apply_scale(g_scale_x, factor);
    else if (axis == 1)
      apply_scale(g_scale_y, factor);
    else
      apply_scale(g_scale_z, factor);
    return true;
  }

  request_redraw();
  return true;
}

void apply_model_transform()
{
  // A ordem das chamadas gera a matriz T * Rx * Ry * Rz * S.
  // Portanto, no vértice, a escala é aplicada primeiro, seguida das rotações
  // e, por último, da translação.
  glTranslatef(g_translate_x, g_translate_y, g_translate_z);
  glRotatef(g_model_rot_x, 1.0f, 0.0f, 0.0f);
  glRotatef(g_model_rot_y, 0.0f, 1.0f, 0.0f);
  glRotatef(g_model_rot_z, 0.0f, 0.0f, 1.0f);
  glScalef(g_scale_x, g_scale_y, g_scale_z);
}

Vec3 inverse_transform_point(const Vec3 &point)
{
  // Inversa de T * Rx * Ry * Rz * S:
  // S^-1 * Rz^-1 * Ry^-1 * Rx^-1 * T^-1.
  Vec3 p(point.x - g_translate_x, point.y - g_translate_y,
         point.z - g_translate_z);
  p = rotate_x(p, -g_model_rot_x);
  p = rotate_y(p, -g_model_rot_y);
  p = rotate_z(p, -g_model_rot_z);
  return Vec3(p.x / g_scale_x, p.y / g_scale_y, p.z / g_scale_z);
}

Vec3 inverse_transform_direction(const Vec3 &direction)
{
  Vec3 d = rotate_x(direction, -g_model_rot_x);
  d = rotate_y(d, -g_model_rot_y);
  d = rotate_z(d, -g_model_rot_z);
  d = Vec3(d.x / g_scale_x, d.y / g_scale_y, d.z / g_scale_z);
  return d;
}

Vec3 transform_model_point(const Vec3 &point)
{
  Vec3 p(point.x * g_scale_x, point.y * g_scale_y, point.z * g_scale_z);
  p = rotate_z(p, g_model_rot_z);
  p = rotate_y(p, g_model_rot_y);
  p = rotate_x(p, g_model_rot_x);
  return Vec3(p.x + g_translate_x, p.y + g_translate_y,
              p.z + g_translate_z);
}
