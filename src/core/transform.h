#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "core/common.h"

// Modos de edição do modelo. As teclas especiais (setas e Page Up/Down)
// executam uma transformação diferente conforme o modo selecionado.
enum TransformMode
{
    TRANSFORM_TRANSLATE,
    TRANSFORM_ROTATE,
    TRANSFORM_SCALE
};

extern float g_translate_x, g_translate_y, g_translate_z;
extern float g_model_rot_x, g_model_rot_y, g_model_rot_z;
extern float g_scale_x, g_scale_y, g_scale_z;
extern TransformMode g_transform_mode;

// Teclas comuns:
// 1 = translação, 2 = rotação, 3 = escala, R = restaurar.
// X/x, Y/y e Z/z continuam esticando/encolhendo diretamente por eixo.
bool handle_transform_key(unsigned char key);

// Setas alteram X/Y e Page Up/Page Down alteram Z no modo atual.
bool handle_transform_special_key(int key);

// Aplica a transformação exclusiva do modelo na matriz MODELVIEW atual.
void apply_model_transform();

// Volta o modelo para posição, rotação e escala originais.
void reset_model_transform();

// Nome do modo atual, usado na interface.
const char *transform_mode_name();

// Conversões utilizadas pelo ray casting e pela futura exportação do STL.
Vec3 inverse_transform_point(const Vec3 &point);
Vec3 inverse_transform_direction(const Vec3 &direction);
Vec3 transform_model_point(const Vec3 &point);

#endif
