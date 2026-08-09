#ifndef UI_H
#define UI_H

// Módulo da interface em tela (HUD): desenha botões e textos diretamente em
// coordenadas de tela, sobre a cena 3D, usando projeção ortográfica.

// Desenha um botão retangular com rótulo centralizado. O parâmetro hover
// controla a cor de destaque quando o cursor está sobre o botão.
void draw_button(int x, int y, int w, int h, const char *label, bool hover);

// Desenha a linha de informações no rodapé (contagem de triângulos, dicas de
// uso e valores atuais de escala).
void draw_info_text();

#endif
