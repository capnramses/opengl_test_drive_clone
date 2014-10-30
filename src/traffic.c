#include "traffic.h"
#include "obj_parser.h"
#include "gl_utils.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define TRUCK_MESH "meshes/truck.obj"
#define TRUCK_WHEEL_MESH "meshes/truck_wheel.obj"
#define TRUCK_DIFF "textures/truck.png"
#define TRUCK_WHEEL_DIFF "textures/truck_wheel.png"
#define TRUCK_VS "shaders/truck.vert"
#define TRUCK_FS "shaders/truck.frag"
#define MAX_TRUCKS 16

vec3 truck_positions[MAX_TRUCKS];
GLuint truck_vao, truck_wheel_vao;
int truck_point_count, truck_wheel_point_count;
int num_trucks;
GLuint truck_sp;
GLint truck_M_loc, truck_V_loc, truck_P_loc;
GLuint truck_diff_map, truck_wheel_diff_map;
float wheel_rot_speed = 500.0f;
float curr_wheel_rot = 0.0f;

bool init_traffic () {
	float* points = NULL;
	float* tex_coords = NULL;
	float* normals = NULL;
	GLuint vp_vbo, vt_vbo, vn_vbo;

	printf ("init traffic...\n");
	
	if (!load_obj_file (
		TRUCK_MESH,
		points,
		tex_coords,
		normals,
		truck_point_count
	)) {
		fprintf (stderr, "ERROR loading truck mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, truck_point_count * sizeof (float) * 3, points,
		GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, truck_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, truck_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
	
	free (points);
	free (tex_coords);
	free (normals);
	
	glGenVertexArrays (1, &truck_vao);
	glBindVertexArray (truck_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	// wheel
	if (!load_obj_file (
		TRUCK_WHEEL_MESH,
		points,
		tex_coords,
		normals,
		truck_wheel_point_count
	)) {
		fprintf (stderr, "ERROR loading truck wheel mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, truck_wheel_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, truck_wheel_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, truck_wheel_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
		
	glGenVertexArrays (1, &truck_wheel_vao);
	glBindVertexArray (truck_wheel_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	// shader
	truck_sp = link_programme_from_files (TRUCK_VS, TRUCK_FS);
	truck_P_loc = glGetUniformLocation (truck_sp, "P");
	truck_V_loc = glGetUniformLocation (truck_sp, "V");
	truck_M_loc = glGetUniformLocation (truck_sp, "M");
	
	truck_diff_map = create_texture_from_file (TRUCK_DIFF);
	truck_wheel_diff_map = create_texture_from_file (TRUCK_WHEEL_DIFF);
	
	add_truck (vec3 (-1.0, 0.0, 0.0));
	add_truck (vec3 (-1.0, 0.0, -10.0));
	add_truck (vec3 (-1.0, 0.0, -15.0));
	
	return true;
}

bool add_truck (vec3 start_pos) {
	assert (num_trucks < MAX_TRUCKS);
	truck_positions[num_trucks] = start_pos;
	num_trucks++;
	return true;
}

bool update_traffic (double elapsed) {
	// rotate truck wheels
	curr_wheel_rot += (float)elapsed * wheel_rot_speed;
	return true;
}

bool draw_traffic () {
	int i;
	
	glUseProgram (truck_sp);
	if (cam_V_dirty) {
		glUniformMatrix4fv (truck_V_loc, 1, GL_FALSE, V.m);
	}
	if (cam_P_dirty) {
		glUniformMatrix4fv (truck_P_loc, 1, GL_FALSE, P.m);
	}
	for (i = 0; i < num_trucks; i++) {
		mat4 M;
		
		// truck chassis
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, truck_diff_map);
		glBindVertexArray (truck_vao);
		M = translate (identity_mat4 (), truck_positions[i]);
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_point_count);
		
		// add 4 wheels
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, truck_wheel_diff_map);
		glBindVertexArray (truck_wheel_vao);
		M = rotate_x_deg (identity_mat4 (), curr_wheel_rot);
		M = translate (M, truck_positions[i] + vec3 (-0.4f, 0.15, 0.45));
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_wheel_point_count);
		
		glBindVertexArray (truck_wheel_vao);
		M = rotate_x_deg (identity_mat4 (), curr_wheel_rot);
		M = translate (M, truck_positions[i] + vec3 (0.4f, 0.15, 0.45));
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_wheel_point_count);
		
		glBindVertexArray (truck_wheel_vao);
		M = rotate_x_deg (identity_mat4 (), curr_wheel_rot);
		M = translate (M, truck_positions[i] + vec3 (-0.45f, 0.15, -1.6));
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_wheel_point_count);
		
		glBindVertexArray (truck_wheel_vao);
		M = rotate_x_deg (identity_mat4 (), curr_wheel_rot);
		M = translate (M, truck_positions[i] + vec3 (0.45f, 0.15, -1.6));
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_wheel_point_count);
	}
	
	return true;
}
