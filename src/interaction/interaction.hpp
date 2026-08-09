#ifndef INTERACTION_H
#define INTERACTION_H

// Módulo de interação com o mouse: converte coordenadas de tela em informações
// do mundo 3D (projeção/unprojeção) para seleção e arraste de objetos da cena.

#include "core/common.hpp"

// Identifica qual lâmpada está sob o cursor (por proximidade na tela).
// Retorna o índice em g_lamp_positions ou -1 se nenhuma for alcançada.
int pick_lamp(int mx, int my);

// Retorna a direção para onde a câmera está olhando no espaço do mundo.
Vec3 get_view_direction();

// Converte a posição do mouse em um ponto 3D sobre o plano definido por
// plane_normal/plane_pt. Usado para arrastar lâmpadas no plano do olhar.
Vec3 mouse_to_3d_plane(int mx, int my, const Vec3 &plane_normal,
                       const Vec3 &plane_pt);

#endif
