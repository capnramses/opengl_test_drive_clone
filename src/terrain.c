#include "terrain.h"
#include "gl_utils.h"
#include "camera.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define ROAD_VS "shaders/road.vert"
#define ROAD_FS "shaders/road.frag"
#define ROAD_TEX "textures/road_seg.png"
#define RCLIFF_VS "shaders/rcliff.vert"
#define RCLIFF_FS "shaders/rcliff.frag"

// VAOs pointing to generated mesh data
GLuint road_vao, rcliff_vao;
// vertex point counts for each VAO
GLuint road_point_count, rcliff_point_count;
// shader programmes
GLuint road_sp, rcliff_sp;
// uniform locations
GLint road_P_loc, road_V_loc;
GLint rcliff_P_loc, rcliff_V_loc, rcliff_w_loc, rcliff_h_loc;
// textures
GLuint road_tex;

// length of each terrain block
unsigned int num_terrain_segs = 10;

//
// pre-generate as much as possible
void init_terrain () {
	printf ("Init terrain...\n");
	srand (time (NULL));
	road_point_count = num_terrain_segs * 6;
	rcliff_point_count = num_terrain_segs * 12;
	
	road_tex = create_texture_from_file (ROAD_TEX);
	
	road_sp = link_programme_from_files (ROAD_VS, ROAD_FS);
	road_P_loc = glGetUniformLocation (road_sp, "P");
	road_V_loc = glGetUniformLocation (road_sp, "V");
	
	rcliff_sp = link_programme_from_files (RCLIFF_VS, RCLIFF_FS);
	rcliff_P_loc = glGetUniformLocation (rcliff_sp, "P");
	rcliff_V_loc = glGetUniformLocation (rcliff_sp, "V");
	rcliff_h_loc = glGetUniformLocation (rcliff_sp, "h");
	rcliff_w_loc = glGetUniformLocation (rcliff_sp, "w");
	
	glGenVertexArrays (1, &road_vao);
	glGenVertexArrays (1, &rcliff_vao);
	
	// gen some terrain blocks
	gen_terrain_block (vec3 (0.0, 0.0, 0.0));
}

//
// generate a random x-offset for a road segment
float rand_road_offs () {
	float o = (float)rand () / (float)RAND_MAX;
	o *= 1.0 - 0.5; // -0.1 to + 0.1
	return o;
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
	float* rcliff_vps = NULL;
	GLuint vp_vbo, vt_vbo, rcliff_vp_vbo;
	GLuint vps_sz, vts_sz, rcliff_vps_sz;
	float curr_x, curr_y, curr_z;
	
	curr_x = start_left.v[0];
	curr_y = start_left.v[1];
	curr_z = start_left.v[2];
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &rcliff_vp_vbo);
	
	type = rand () % 4;
	printf ("block type %i gend\n", type);
	
	vps_sz = num_terrain_segs * 6 * 3 * sizeof (float);
	vps = (float*)malloc (vps_sz);
	
	rcliff_vps_sz = num_terrain_segs * 12 * 3 * sizeof (float);
	rcliff_vps = (float*)malloc (rcliff_vps_sz);
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
	// TODO add curve functions to X
	for (i = 0; i < num_terrain_segs; i++) {
		// this is for adding some jitter to the road/cliffs so never perfectly
		// straight
		float x_off = 0.0f;
		float rcliff_middle_x_off = 0.5f;
		float rcliff_top_x_off = 4.0f;
		
		x_off = rand_road_offs ();
		
		//
		// generate the road surface
		
		// point 0
		vps[i * 18 + 0] = curr_x + -2.0f;
		vps[i * 18 + 1] = curr_y + 0.0f;
		vps[i * 18 + 2] = curr_z + -1.0f * (float)(i * 4);
		// point 1
		vps[i * 18 + 3] = curr_x + 2.0f;
		vps[i * 18 + 4] = curr_y + 0.0f;
		vps[i * 18 + 5] = curr_z + -1.0f * (float)(i * 4);
		// point 2
		vps[i * 18 + 6] = curr_x + 2.0f + x_off;
		vps[i * 18 + 7] = curr_y + 0.0f;
		vps[i * 18 + 8] = curr_z + -1.0f * (float)((i + 1) * 4);
		// point 3
		vps[i * 18 + 9] = curr_x + 2.0f + x_off;
		vps[i * 18 + 10] = curr_y + 0.0f;
		vps[i * 18 + 11] = curr_z + -1.0f * (float)((i + 1) * 4);
		// point 4
		vps[i * 18 + 12] = curr_x + -2.0f + x_off;
		vps[i * 18 + 13] = curr_y + 0.0f;
		vps[i * 18 + 14] = curr_z + -1.0f * (float)((i + 1) * 4);
		// point 5
		vps[i * 18 + 15] = curr_x + -2.0f;
		vps[i * 18 + 16] = curr_y + 0.0f;
		vps[i * 18 + 17] = curr_z + -1.0f * (float)(i * 4);
		
		//
		// generate the cliffs on right side
		// road points 1 and 2 are the bottom of the cliff
		
		// point 0 (road point 2 - forward)
		rcliff_vps[i * 36 + 0] = curr_x + 2.0f + x_off;
		rcliff_vps[i * 36 + 1] = curr_y + 0.0f;
		rcliff_vps[i * 36 + 2] = curr_z + -1.0f * (float)((i + 1) * 4);
		// point 1 (road point 1 - back)
		rcliff_vps[i * 36 + 3] = curr_x + 2.0f;
		rcliff_vps[i * 36 + 4] = curr_y + 0.0f;
		rcliff_vps[i * 36 + 5] = curr_z + -1.0f * (float)(i * 4);
		// point 2 (previous point but up 1)
		rcliff_vps[i * 36 + 6] = curr_x + 2.0f + rcliff_middle_x_off;
		rcliff_vps[i * 36 + 7] = curr_y + 2.0f;
		rcliff_vps[i * 36 + 8] = curr_z + -1.0f * (float)(i * 4);
		// point 3 (same again)
		rcliff_vps[i * 36 + 9] = curr_x + 2.0f + rcliff_middle_x_off;
		rcliff_vps[i * 36 + 10] = curr_y + 2.0f;
		rcliff_vps[i * 36 + 11] = curr_z + -1.0f * (float)(i * 4);
		// point 4 (previous point but forward)
		rcliff_vps[i * 36 + 12] = curr_x + 2.0f + x_off + rcliff_middle_x_off;
		rcliff_vps[i * 36 + 13] = curr_y + 2.0f;
		rcliff_vps[i * 36 + 14] = curr_z + -1.0f * (float)((i + 1) * 4);
		// point 5 (back to the zeroth point)
		rcliff_vps[i * 36 + 15] = curr_x + 2.0f + x_off;
		rcliff_vps[i * 36 + 16] = curr_y + 0.0f;
		rcliff_vps[i * 36 + 17] = curr_z + -1.0f * (float)((i + 1) * 4);
		
		//
		// 2nd row on right-hand cliff
		
		// point 6 (road point 2 - forward)
		rcliff_vps[i * 36 + 18] = curr_x + 2.0f + x_off + rcliff_middle_x_off;
		rcliff_vps[i * 36 + 19] = curr_y + 2.0f;
		rcliff_vps[i * 36 + 20] = curr_z + -1.0f * (float)((i + 1) * 4);
		// point 7 (road point 1 - back)
		rcliff_vps[i * 36 + 21] = curr_x + 2.0f + rcliff_middle_x_off;
		rcliff_vps[i * 36 + 22] = curr_y + 2.0f;
		rcliff_vps[i * 36 + 23] = curr_z + -1.0f * (float)(i * 4);
		// point 8 (previous point but up 1)
		rcliff_vps[i * 36 + 24] = curr_x + 2.0f + rcliff_top_x_off;
		rcliff_vps[i * 36 + 25] = curr_y + 12.0f;
		rcliff_vps[i * 36 + 26] = curr_z + -1.0f * (float)(i * 4);
		// point 9 (same again)
		rcliff_vps[i * 36 + 27] = curr_x + 2.0f + rcliff_top_x_off;
		rcliff_vps[i * 36 + 28] = curr_y + 12.0f;
		rcliff_vps[i * 36 + 29] = curr_z + -1.0f * (float)(i * 4);
		// point 10 (previous point but forward)
		rcliff_vps[i * 36 + 30] = curr_x + 2.0f + x_off + rcliff_top_x_off;
		rcliff_vps[i * 36 + 31] = curr_y + 12.0f;
		rcliff_vps[i * 36 + 32] = curr_z + -1.0f * (float)((i + 1) * 4);
		// point 11 (back to the zeroth point)
		rcliff_vps[i * 36 + 33] = curr_x + 2.0f + x_off + rcliff_middle_x_off;
		rcliff_vps[i * 36 + 34] = curr_y + 2.0f;
		rcliff_vps[i * 36 + 35] = curr_z + -1.0f * (float)((i + 1) * 4);
		
		curr_x = curr_x + x_off;
		// TODO z as well instead of i *
	}
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, vps_sz, vps, GL_STATIC_DRAW);
	free (vps);
	
	glBindBuffer (GL_ARRAY_BUFFER, rcliff_vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, rcliff_vps_sz, rcliff_vps, GL_STATIC_DRAW);
	free (rcliff_vps);
	
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
	
	glBindVertexArray (road_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glBindVertexArray (rcliff_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, rcliff_vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	return true;
}

void draw_terrain () {
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, road_tex);
	
	glUseProgram (road_sp);
	if (cam_V_dirty) {
		glUniformMatrix4fv (road_V_loc, 1, GL_FALSE, V.m);
	}
	if (cam_P_dirty) {
		glUniformMatrix4fv (road_P_loc, 1, GL_FALSE, P.m);
	}
	
	glBindVertexArray (road_vao);
	glDrawArrays (GL_TRIANGLES, 0, road_point_count);
	
	glUseProgram (rcliff_sp);
	if (cam_V_dirty) {
		glUniformMatrix4fv (rcliff_V_loc, 1, GL_FALSE, V.m);
	}
	if (cam_P_dirty) {
		glUniformMatrix4fv (rcliff_P_loc, 1, GL_FALSE, P.m);
		glUniform1f (rcliff_h_loc, (float)gl_height);
		glUniform1f (rcliff_w_loc, (float)gl_width);
	}
	
	glBindVertexArray (rcliff_vao);
	glDrawArrays (GL_TRIANGLES, 0, rcliff_point_count);
}
