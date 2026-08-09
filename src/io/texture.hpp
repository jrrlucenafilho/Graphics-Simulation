#ifndef TEXTURE_H
#define TEXTURE_H

// Módulo de carregamento de texturas 2D (imagens) usando stb_image,
// integradas ao OpenGL como textura de mapeamento da malha do modelo.

#include <string>

// Carrega a imagem do caminho informado como textura GL e a torna ativa.
// Retorna true em caso de sucesso.
bool load_texture(const std::string &path);

#endif
