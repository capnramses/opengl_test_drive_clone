#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "maths_funcs.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void init_terrain ();
bool gen_terrain_block (vec3 start_left, int vao_index, vec3* end_left);
void draw_terrain ();

#endif
