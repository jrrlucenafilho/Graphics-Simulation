#include "transform.h"
#include "core/common.h"
#include <cstdio>

float g_scale_x = 1.0f, g_scale_y = 1.0f, g_scale_z = 1.0f;

#define SCALE_STEP 1.1f
#define SCALE_MIN 0.1f
#define SCALE_MAX 10.0f

static void apply_scale(float &axis_scale, float factor) {
  float next = axis_scale * factor;
  if (next < SCALE_MIN || next > SCALE_MAX)
    return;
  axis_scale = next;
  printf("Escala: X=%.2f Y=%.2f Z=%.2f\n", g_scale_x, g_scale_y, g_scale_z);
  glutPostRedisplay();
}

bool handle_scale_key(unsigned char key) {
  switch (key) {
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
