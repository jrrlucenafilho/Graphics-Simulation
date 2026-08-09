#ifndef STL_IO_H
#define STL_IO_H

// Módulo de leitura e escrita de arquivos STL, além do pós-processamento da
// malha (centralização, orientação e geração de coordenadas de textura).

#include "../core/common.hpp"
#include <string>

// Carrega um arquivo STL (ASCII ou binário) para g_triangles.
// Retorna true em caso de sucesso. Não centraliza nem orienta o modelo;
// isso é feito separadamente por center_model()/orient_model().
bool load_stl(const std::string &path);

// Carrega um arquivo OBJ (Wavefront) com textura para g_triangles: lê a
// geometria, os UVs e, se o arquivo .mtl referenciar uma textura, carrega-a.
bool load_obj(const std::string &path);

// Carrega um modelo escolhendo o formato pela extensão do arquivo: .obj usa
// load_obj (mantém orientação/UVs do arquivo e carrega a textura); outros
// formatos usam load_stl seguido de orient_model() e generate_uv_coords().
bool load_model(const std::string &path);

// Exporta a malha atual como STL ASCII, já com a escala aplicada.
bool export_stl_ascii(const std::string &path);

// Exporta a malha atual como Wavefront OBJ com textura: gera o arquivo .obj,
// o .mtl (que referencia a textura) e copia a textura para o mesmo diretório.
// Só deve ser chamado quando g_texture_loaded é true.
bool export_obj(const std::string &path);

// Centraliza o modelo na origem e o normaliza para caber no intervalo [-1, 1],
// garantindo que modelos de tamanhos diferentes apareçam na mesma escala.
void center_model();

// Ajusta a orientação do modelo: arquivos STL (CAD/impressão 3D) usam Z como
// eixo vertical, enquanto a cena usa Y; esta rotação deixa o modelo em pé e
// com a frente voltada para o observador (+Z).
void orient_model();

// Gera coordenadas de textura (u, v) por vértice a partir da posição do
// vértice, normalizadas pelo maior eixo do modelo.
void generate_uv_coords();

#endif
