#ifndef TRANSFORM_H
#define TRANSFORM_H

// Módulo de transformação de escala do modelo.
//
// Escala não uniforme do modelo (esticar/encolher por eixo). A escala é
// aplicada na renderização via glScalef (matriz de transformação); a geometria
// em memória não é alterada, o que evita recomputar normais a cada tecla.

// Trata as teclas X/x, Y/y e Z/z; retorna true se a tecla foi consumida.
// Letras maiúsculas aumentam a escala do respectivo eixo; minúsculas reduzem.
bool handle_scale_key(unsigned char key);

#endif
