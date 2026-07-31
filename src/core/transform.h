#ifndef TRANSFORM_H
#define TRANSFORM_H

// Escala não uniforme do modelo (esticar/encolher por eixo).
// A escala é aplicada na renderização via glScalef (matriz de transformação);
// a geometria em memória não é alterada.

// Trata as teclas X/x, Y/y e Z/z; retorna true se a tecla foi consumida.
bool handle_scale_key(unsigned char key);

#endif
