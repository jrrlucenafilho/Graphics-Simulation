#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include "stb_image.h"
#include <cstdio>

bool load_texture(const std::string &path) {
  int w, h, channels;
  unsigned char *data = stbi_load(path.c_str(), &w, &h, &channels, 3);
  if (!data) {
    printf("Erro ao carregar textura: %s\n", path.c_str());
    return false;
  }

  if (g_texture_loaded)
    glDeleteTextures(1, &g_texture_id);

  glGenTextures(1, &g_texture_id);
  glBindTexture(GL_TEXTURE_2D, g_texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,
               data);

  stbi_image_free(data);
  g_texture_loaded = true;
  printf("Textura carregada: %s (%dx%d)\n", path.c_str(), w, h);
  return true;
}
