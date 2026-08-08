#include "transform.h"
#include "core/common.h"
#include <cstdio>

// Definição das variáveis globais de escala (declaradas como extern em common.h).
// A escala inicia em 1.0 (sem alteração) e é usada na renderização via glScalef.
float g_scale_x = 1.0f, g_scale_y = 1.0f, g_scale_z = 1.0f;

// Limites e passo da escala aplicados a cada eixo. O passo multiplicativo
// (1.1x) dá uma sensação progressiva de zoom; os limites evitam que o modelo
// fique insignificante ou gigante demais.
#define SCALE_STEP 1.1f
#define SCALE_MIN 0.1f
#define SCALE_MAX 10.0f

// Aplica um fator multiplicativo ao valor de escala de um eixo. Se o resultado
// sair dos limites aceitáveis, a alteração é ignorada; caso contrário atualiza
// o valor e solicita o redesenho da cena.
static void apply_scale(float &axis_scale, float factor) {
  float next = axis_scale * factor;
  if (next < SCALE_MIN || next > SCALE_MAX)
    return;
  axis_scale = next;
  printf("Escala: X=%.2f Y=%.2f Z=%.2f\n", g_scale_x, g_scale_y, g_scale_z);
  glutPostRedisplay();
}

// Mapeia teclas de escala. As letras maiúsculas (X, Y, Z) aumentam o eixo
// correspondente e as minúsculas (x, y, z) diminuem. Retorna true quando a
// tecla é tratada aqui, para que o main não a processe como outra função.
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
