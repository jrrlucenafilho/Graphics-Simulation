#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H

// Módulo de diálogos de seleção de arquivo. Usa o utilitário zenity (GTK)
// via linha de comando, comum em distribuições Linux, para abrir as janelas
// nativas de abrir/salvar arquivos.

#include <string>

// Abre o diálogo "abrir arquivo" filtrado para STL. Retorna o caminho
// escolhido ou string vazia se o usuário cancelar.
std::string open_file_dialog();

// Abre o diálogo "salvar arquivo" para exportação STL, adicionando a extensão
// .stl quando o usuário não a informa.
std::string save_file_dialog();

// Abre o diálogo "abrir arquivo" filtrado para imagens de textura.
std::string open_texture_dialog();

#endif
