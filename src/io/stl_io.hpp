#ifndef STL_IO_HPP
#define STL_IO_HPP

#include <string>
#include "core/common.hpp"

bool load_stl(const std::string &path);
bool export_stl_ascii(const std::string &path);
void center_model();
void generate_uv_coords();

#endif
