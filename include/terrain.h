#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void init_terrain ();
bool gen_terrain_block (float back_left_x, float back_left_y, GLuint vbo);

#endif
