#define STB_IMAGE_IMPLEMENTATION
#include "texture.hpp"
#include "stb_image.h"
#include <cstdio>

// Carrega uma imagem (JPG/PNG/etc.) como textura OpenGL.
// stb_image decodifica a imagem em pixels RGB, que são enviados à GPU com
// glTexImage2D. Parâmetros de filtro e repetição controlam como a textura é
// amostrada sobre a malha.
bool load_texture(const std::string &path) {
  int w, h, channels;
  // "3" no final força a decodificação para 3 canais (RGB).
  unsigned char *data = stbi_load(path.c_str(), &w, &h, &channels, 3);
  if (!data) {
    printf("Erro ao carregar textura: %s\n", path.c_str());
    return false;
  }

  // Se já havia uma textura, libera-a para reutilizar o identificador.
  if (g_texture_loaded)
    glDeleteTextures(1, &g_texture_id);

  // Cria e ativa a nova textura.
  glGenTextures(1, &g_texture_id);
  glBindTexture(GL_TEXTURE_2D, g_texture_id);

  // Configura repetição (GL_REPEAT) e interpolação linear (GL_LINEAR), o que
  // evita artefatos ao ampliar/reduzir a textura sobre os triângulos.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Transfere os pixels da CPU para a memória da GPU.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,
               data);

  stbi_image_free(data);
  g_texture_loaded = true;
  g_texture_path = path;
  printf("Textura carregada: %s (%dx%d)\n", path.c_str(), w, h);
  return true;
}
