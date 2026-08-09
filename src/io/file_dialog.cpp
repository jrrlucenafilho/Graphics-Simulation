#include "file_dialog.hpp"
#include <cstdio>
#include <cstdlib>

// Executa o comando zenity via popen, que abre a janela de seleção nativa e
// devolve o caminho escolhido na saída padrão. O caminho é lido, limpo de
// quebras de linha e retornado; string vazia significa cancelamento.

std::string open_file_dialog() {
  std::string cmd =
      "zenity --file-selection --title=\"Selecionar modelo 3D\" "
      "--file-filter=\"Modelos 3D (*.stl *.obj) | *.stl *.obj\" 2>/dev/null";
  FILE *fp = popen(cmd.c_str(), "r");
  if (!fp)
    return "";
  char buf[4096] = {0};
  if (fgets(buf, sizeof(buf) - 1, fp)) {
    pclose(fp);
    std::string s(buf);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
      s.pop_back();
    return s;
  }
  pclose(fp);
  return "";
}

// Igual ao de abrir, porém com o modo "salvar" e um nome sugerido. O formato
// sugerido/filtrado depende de haver textura: .obj quando with_texture é true,
// .stl caso contrário. Se o usuário não digitar a extensão, ela é acrescentada
// automaticamente.
std::string save_file_dialog(bool with_texture) {
  std::string cmd;
  if (with_texture) {
    cmd = "zenity --file-selection --save --title=\"Exportar modelo OBJ\" "
          "--file-filter=\"OBJ files (*.obj) | *.obj\" "
          "--filename=\"modelo_exportado.obj\" 2>/dev/null";
  } else {
    cmd = "zenity --file-selection --save --title=\"Exportar modelo STL\" "
          "--file-filter=\"STL files (*.stl) | *.stl\" "
          "--filename=\"modelo_exportado.stl\" 2>/dev/null";
  }
  FILE *fp = popen(cmd.c_str(), "r");
  if (!fp)
    return "";
  char buf[4096] = {0};
  if (fgets(buf, sizeof(buf) - 1, fp)) {
    pclose(fp);
    std::string s(buf);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
      s.pop_back();
    if (s.find('.') == std::string::npos)
      s += with_texture ? ".obj" : ".stl";
    return s;
  }
  pclose(fp);
  return "";
}

// Diálogo de seleção de imagem (textura), filtrado para formatos comuns.
std::string open_texture_dialog() {
  std::string cmd =
      "zenity --file-selection --title=\"Selecionar textura\" "
      "--file-filter=\"Imagens (*.jpg *.jpeg *.png) | *.jpg *.jpeg *.png\" "
      "2>/dev/null";
  FILE *fp = popen(cmd.c_str(), "r");
  if (!fp)
    return "";
  char buf[4096] = {0};
  if (fgets(buf, sizeof(buf) - 1, fp)) {
    pclose(fp);
    std::string s(buf);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
      s.pop_back();
    return s;
  }
  pclose(fp);
  return "";
}
