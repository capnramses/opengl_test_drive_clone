#include "terrain.h"
#include "gl_utils.h"
#include "camera.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define TERRAIN_VS "shaders/test.vert"
#define TERRAIN_FS "shaders/test.frag"
#define TERRAIN_TEX "textures/road_seg.png"

GLuint terrain_vao;
GLuint terrain_sp;
GLint terrain_P_loc;
GLint terrain_V_loc;
GLuint terrain_tex;

// length of each terrain block
unsigned int num_terrain_segs = 10;
unsigned int terrain_point_count;

//
// pre-generate as much as possible
void init_terrain () {
	printf ("Init terrain...\n");
	srand (time (NULL));
	terrain_point_count = num_terrain_segs * 6;
	
	terrain_tex = create_texture_from_file (TERRAIN_TEX);
	
	terrain_sp = link_programme_from_files (TERRAIN_VS, TERRAIN_FS);
	terrain_P_loc = glGetUniformLocation (terrain_sp, "P");
	terrain_V_loc = glGetUniformLocation (terrain_sp, "V");
	
	glGenVertexArrays (1, &terrain_vao);
	
	// gen some terrain blocks
	gen_terrain_block (vec3 (-2.0, 0.0, 0.0));
}

//
// creates new randomised geometry and stores in an existing vertex buffer
// object
bool gen_terrain_block (vec3 start_left) {
	// decide on type
	// 0 - straight
	// 1 - curve left
	// 2 - curve right
	// 3 - double curve
	// all types have random variation thrown in
	int type = 0;
	unsigned int i = 0;
	float* vps = NULL;
	float* vts = NULL;
	GLuint vp_vbo, vt_vbo;
	GLuint vps_sz, vts_sz;
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	type = rand () % 4;
	printf ("block type %i gend\n", type);
	
	vps_sz = num_terrain_segs * 6 * 3 * sizeof (float);
	vps = (float*)malloc (vps_sz);
	// create 2 triangle for each seg, starting at back_left, and finishing at
	// top_left
	
/*
Road segment created in buffer like this (birds-eye view of road):

4    2,3
------
|   /|
|  / |
| /  |
|/___|
5,0  1

*/
	// TODO use start_left here
	// TODO add random variation to X
	// TODO add curve functions to X
	for (i = 0; i < num_terrain_segs; i++) {
		// point 0
		vps[i * 18 + 0] = -2.0f;
		vps[i * 18 + 1] = 0.0f;
		vps[i * 18 + 2] = -1.0f * (float)(i * 4);
		// point 1
		vps[i * 18 + 3] = 2.0f;
		vps[i * 18 + 4] = 0.0f;
		vps[i * 18 + 5] = -1.0f * (float)(i * 4);
		// point 2
		vps[i * 18 + 6] = 2.0f;
		vps[i * 18 + 7] = 0.0f;
		vps[i * 18 + 8] = -1.0f * (float)((i + 1) * 4);
		// point 3
		vps[i * 18 + 9] = 2.0f;
		vps[i * 18 + 10] = 0.0f;
		vps[i * 18 + 11] = -1.0f * (float)((i + 1) * 4);
		// point 4
		vps[i * 18 + 12] = -2.0f;
		vps[i * 18 + 13] = 0.0f;
		vps[i * 18 + 14] = -1.0f * (float)((i + 1) * 4);
		// point 5
		vps[i * 18 + 15] = -2.0f;
		vps[i * 18 + 16] = 0.0f;
		vps[i * 18 + 17] = -1.0f * (float)(i * 4);
	}
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, vps_sz, vps, GL_STATIC_DRAW);
	free (vps);
	
	vts_sz = num_terrain_segs * 6 * 2 * sizeof (float);
	vts = (float*)malloc (vts_sz);
	for (i = 0; i < num_terrain_segs; i++) {
		// point 0
		vts[i * 12 + 0] = 0.0f;
		vts[i * 12 + 1] = 0.0f;
		// point 1
		vts[i * 12 + 2] = 1.0f;
		vts[i * 12 + 3] = 0.0f;
		// point 2
		vts[i * 12 + 4] = 1.0f;
		vts[i * 12 + 5] = 1.0f;
		// point 3
		vts[i * 12 + 6] = 1.0f;
		vts[i * 12 + 7] = 1.0f;
		// point 4
		vts[i * 12 + 8] = 0.0f;
		vts[i * 12 + 9] = 1.0f;
		// point 5
		vts[i * 12 + 10] = 0.0f;
		vts[i * 12 + 11] = 0.0f;
	}
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, vts_sz, vts, GL_STATIC_DRAW);
	free (vts);
	
	glBindVertexArray (terrain_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	
	return true;
}

void draw_terrain () {
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, terrain_tex);
	
	glUseProgram (terrain_sp);
	if (cam_V_dirty) {
		glUniformMatrix4fv (terrain_V_loc, 1, GL_FALSE, V.m);
	}
	if (cam_P_dirty) {
		glUniformMatrix4fv (terrain_P_loc, 1, GL_FALSE, P.m);
	}
	
	glBindVertexArray (terrain_vao);
	glDrawArrays (GL_TRIANGLES, 0, terrain_point_count);
}
