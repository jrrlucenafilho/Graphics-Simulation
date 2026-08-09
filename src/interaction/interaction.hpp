#ifndef INTERACTION_HPP
#define INTERACTION_HPP

#include "core/common.hpp"

int pick_triangle(int mx, int my);
int pick_lamp(int mx, int my);
Vec3 get_view_direction();
Vec3 mouse_to_3d_plane(int mx, int my, const Vec3 &plane_normal,
                        const Vec3 &plane_pt);

#endif
