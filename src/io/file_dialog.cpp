#include "file_dialog.h"
#include <cstdio>
#include <cstdlib>

std::string open_file_dialog() {
  std::string cmd = "zenity --file-selection --title=\"Selecionar modelo STL\" "
                    "--file-filter=\"STL files (*.stl) | *.stl\" 2>/dev/null";
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

std::string save_file_dialog() {
  std::string cmd =
      "zenity --file-selection --save --title=\"Exportar modelo STL\" "
      "--file-filter=\"STL files (*.stl) | *.stl\" "
      "--filename=\"modelo_exportado.stl\" 2>/dev/null";
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
      s += ".stl";
    return s;
  }
  pclose(fp);
  return "";
}

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
