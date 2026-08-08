#ifndef STL_IO_H
#define STL_IO_H

#include <string>
#include "core/common.h"

bool load_stl(const std::string &path);
bool export_stl_ascii(const std::string &path);
void center_model();
void orient_model();
void generate_uv_coords();

#endif
