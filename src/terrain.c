#include "terrain.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//
// pre-generate as much as possible
void init_terrain () {
	printf ("Init terrain...\n");
	srand (time (NULL));
	
	// gen some terrain blocks
	gen_terrain_block (0.0f, 0.0f, 0);
	gen_terrain_block (0.0f, 0.0f, 0);
	gen_terrain_block (0.0f, 0.0f, 0);
	gen_terrain_block (0.0f, 0.0f, 0);
	gen_terrain_block (0.0f, 0.0f, 0);
	gen_terrain_block (0.0f, 0.0f, 0);
	gen_terrain_block (0.0f, 0.0f, 0);
	gen_terrain_block (0.0f, 0.0f, 0);
}

//
// creates new randomised geometry and stores in an existing vertex buffer
// object
bool gen_terrain_block (float back_left_x, float back_left_y, GLuint vbo) {
	// decide on type
	// 0 - straight
	// 1 - curve left
	// 2 - curve right
	// 3 - double curve
	// all types have random variation thrown in
	int type = 0;
	
	type = rand () % 4;
	printf ("block type %i gend\n", type);
}
