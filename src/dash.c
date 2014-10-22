#include "dash.h"
#include "obj_parser.h"
#include "gl_utils.h"
#include "camera.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#define DASH_MESH "meshes/dash.obj"
#define DASH_VS "shaders/dash.vert"
#define DASH_FS "shaders/dash.frag"

GLuint dash_vao;
int dash_point_count;
GLuint dash_sp;
GLint dash_M_loc, dash_V_loc;
GLint dash_P_loc;
GLint dash_w_loc, dash_h_loc;
mat4 dash_M;

bool init_dash () {
	float* points = NULL;
	float* tex_coords = NULL;
	float* normals = NULL;
	GLuint vp_vbo, vt_vbo, vn_vbo;

	printf ("Init dashboard...\n");
	
	dash_M = identity_mat4 ();
	
	if (!load_obj_file (
		DASH_MESH,
		points,
		tex_coords,
		normals,
		dash_point_count
	)) {
		fprintf (stderr, "ERROR loading dashboard mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, dash_point_count * sizeof (float) * 3, points,
		GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, dash_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, dash_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
	
	free (points);
	free (tex_coords);
	free (normals);
	
	glGenVertexArrays (1, &dash_vao);
	glBindVertexArray (dash_vao);
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
	dash_sp = link_programme_from_files (DASH_VS, DASH_FS);
	dash_P_loc = glGetUniformLocation (dash_sp, "P");
	dash_V_loc = glGetUniformLocation (dash_sp, "V");
	dash_M_loc = glGetUniformLocation (dash_sp, "M");
	dash_w_loc = glGetUniformLocation (dash_sp, "w");
	dash_h_loc = glGetUniformLocation (dash_sp, "h");
	
	return true;
}

void move_dash (vec3 p) {
	dash_M = translate (identity_mat4 (), p + vec3 (0.0f, 0.0f, -1.125f));
}

void draw_dash () {
	glUseProgram (dash_sp);
	if (cam_V_dirty) {
		glUniformMatrix4fv (dash_V_loc, 1, GL_FALSE, V.m);
	}
	if (cam_P_dirty) {
		glUniformMatrix4fv (dash_P_loc, 1, GL_FALSE, P.m);
		glUniform1f (dash_h_loc, (float)gl_height);
		glUniform1f (dash_w_loc, (float)gl_width);
	}
	
	glUniformMatrix4fv (dash_M_loc, 1, GL_FALSE, dash_M.m);
	glBindVertexArray (dash_vao);
	glDrawArrays (GL_TRIANGLES, 0, dash_point_count);
}
