#include "dash.h"
#include "obj_parser.h"
#include "gl_utils.h"
#include "camera.h"
#include "stb_image.h"
#include "player.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#define DASH_MESH "meshes/dash.obj"
#define MIRROR_MESH "meshes/mirror.obj"
#define STEERING_MESH "meshes/steering.obj"
#define SPEEDO_MESH "meshes/speedo.obj"
#define NEEDLE_MESH "meshes/needle.obj"
#define DASH_VS "shaders/dash.vert"
#define DASH_FS "shaders/dash.frag"
#define MIRROR_VS "shaders/mirror.vert"
#define MIRROR_FS "shaders/mirror.frag"
#define MIRROR_OUTER_VS "shaders/mirror_outer.vert"
#define MIRROR_OUTER_FS "shaders/mirror_outer.frag"
#define STEERING_VS "shaders/steering.vert"
#define STEERING_FS "shaders/steering.frag"
#define SMASHED_VS "shaders/smashed.vert"
#define SMASHED_FS "shaders/smashed.frag"
#define SPEEDO_VS "shaders/speedo.vert"
#define SPEEDO_FS "shaders/speedo.frag"
#define TACHO_VS "shaders/tacho.vert"
#define TACHO_FS "shaders/tacho.frag"
#define NEEDLE_VS "shaders/needle.vert"
#define NEEDLE_FS "shaders/needle.frag"
#define GEAR_VS "shaders/gear.vert"
#define GEAR_FS "shaders/gear.frag"
#define STEERING_DIFF "textures/steering.png"
#define SMASHED_DIFF "textures/smashed.png"
#define TACHO_FULL_DIFF "textures/tacho_full.png"
#define TACHO_EMPTY_DIFF "textures/tacho_empty.png"
#define SPEEDO_DIFF "textures/speedo.png"

GLuint dash_vao, mirror_vao, steering_vao, speedo_vao, needle_vao;
int dash_point_count, mirror_point_count, steering_point_count,
	speedo_point_count, needle_point_count;

GLuint dash_sp, mirror_sp, mirror_outer_sp, steering_sp, smashed_sp, tacho_sp,
	speedo_sp, needle_sp, gear_sp;
GLint dash_M_loc, dash_V_loc, dash_P_loc, dash_w_loc, dash_h_loc;
GLint mirror_M_loc, mirror_V_loc,  mirror_P_loc;
GLint mirror_outer_M_loc, mirror_outer_V_loc,  mirror_outer_P_loc;
GLint steering_M_loc, steering_V_loc, steering_P_loc, steering_w_loc,
	steering_h_loc;
GLint tacho_M_loc, tacho_V_loc, tacho_P_loc, tacho_rpm_fac_loc;
GLint gear_M_loc, gear_V_loc, gear_P_loc;
GLint speedo_M_loc, speedo_V_loc, speedo_P_loc;
GLint needle_M_loc, needle_V_loc, needle_P_loc;
GLuint steering_diff_map, smashed_diff_map, tacho_full_diff_map, gear_dms[6],
	tacho_empty_diff_map, speedo_diff_map;
float steering_deg;

mat4 dash_M, tacho_M, mirror_M, mirror_outer_M, steering_M, dash_V, P_boring,
	speedo_M, needle_M, gear_M;
vec3 dash_pos;

bool draw_smashed;

bool init_dash () {
	float* points = NULL;
	float* tex_coords = NULL;
	float* normals = NULL;
	GLuint vp_vbo, vt_vbo, vn_vbo;

	printf ("Init dashboard...\n");
	
	dash_M = identity_mat4 ();
	speedo_M = identity_mat4 ();
	needle_M = identity_mat4 ();
	tacho_M = identity_mat4 ();
	mirror_M = identity_mat4 ();
	mirror_outer_M = identity_mat4 ();
	steering_M = identity_mat4 ();
	gear_M = identity_mat4 ();

	// custom look-at for dashboard
	dash_V = look_at (vec3 (0.0f, 0.0f, 0.0f), vec3 (0.0f, 0.0f, -1.0f),
		vec3 (0.0f, 1.0f, 0.0f));
	
	P_boring = perspective (
			67.0f, (float)gl_width / (float)gl_height, 0.1f, 200.0f);
	
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
	glBufferData (GL_ARRAY_BUFFER, dash_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
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
	
	if (!load_obj_file (
		MIRROR_MESH,
		points,
		tex_coords,
		normals,
		mirror_point_count
	)) {
		fprintf (stderr, "ERROR loading mirror mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, mirror_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, mirror_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, mirror_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
		
	glGenVertexArrays (1, &mirror_vao);
	glBindVertexArray (mirror_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	free (points);
	free (tex_coords);
	free (normals);
	
	//
	// steering wheel
	//
	if (!load_obj_file (
		STEERING_MESH,
		points,
		tex_coords,
		normals,
		steering_point_count
	)) {
		fprintf (stderr, "ERROR loading steering mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, steering_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, steering_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, steering_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
	
	glGenVertexArrays (1, &steering_vao);
	glBindVertexArray (steering_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	if (!load_obj_file (
		SPEEDO_MESH,
		points,
		tex_coords,
		normals,
		speedo_point_count
	)) {
		fprintf (stderr, "ERROR loading speedo mesh\n");
		return false;
	}
	printf ("loaded speedo with %i pts\n", speedo_point_count);
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, speedo_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, speedo_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, speedo_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
	
	free (points);
	free (tex_coords);
	free (normals);
	
	glGenVertexArrays (1, &speedo_vao);
	glBindVertexArray (speedo_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	
	if (!load_obj_file (
		NEEDLE_MESH,
		points,
		tex_coords,
		normals,
		needle_point_count
	)) {
		fprintf (stderr, "ERROR loading needle mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, needle_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, needle_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, needle_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
	
	free (points);
	free (tex_coords);
	free (normals);
	
	glGenVertexArrays (1, &needle_vao);
	glBindVertexArray (needle_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	
	// shader
	dash_sp = link_programme_from_files (DASH_VS, DASH_FS);
	dash_P_loc = glGetUniformLocation (dash_sp, "P");
	dash_V_loc = glGetUniformLocation (dash_sp, "V");
	dash_M_loc = glGetUniformLocation (dash_sp, "M");
	dash_w_loc = glGetUniformLocation (dash_sp, "w");
	dash_h_loc = glGetUniformLocation (dash_sp, "h");
	glUseProgram (dash_sp);
	glUniformMatrix4fv (dash_V_loc, 1, GL_FALSE, dash_V.m);
	uniforms++;
	
	mirror_sp = link_programme_from_files (MIRROR_VS, MIRROR_FS);
	mirror_P_loc = glGetUniformLocation (mirror_sp, "P");
	mirror_V_loc = glGetUniformLocation (mirror_sp, "V");
	mirror_M_loc = glGetUniformLocation (mirror_sp, "M");
	glUseProgram (mirror_sp);
	glUniformMatrix4fv (mirror_V_loc, 1, GL_FALSE, dash_V.m);
	uniforms++;
	
	mirror_outer_sp = link_programme_from_files (
		MIRROR_OUTER_VS, MIRROR_OUTER_FS);
	mirror_outer_P_loc = glGetUniformLocation (mirror_outer_sp, "P");
	mirror_outer_V_loc = glGetUniformLocation (mirror_outer_sp, "V");
	mirror_outer_M_loc = glGetUniformLocation (mirror_outer_sp, "M");
	glUseProgram (mirror_outer_sp);
	glUniformMatrix4fv (mirror_outer_V_loc, 1, GL_FALSE, dash_V.m);
	uniforms++;
	
	steering_sp = link_programme_from_files (STEERING_VS, STEERING_FS);
	steering_P_loc = glGetUniformLocation (steering_sp, "P");
	steering_V_loc = glGetUniformLocation (steering_sp, "V");
	steering_M_loc = glGetUniformLocation (steering_sp, "M");
	steering_w_loc = glGetUniformLocation (steering_sp, "w");
	steering_h_loc = glGetUniformLocation (steering_sp, "h");
	glUseProgram (steering_sp);
	glUniformMatrix4fv (steering_V_loc, 1, GL_FALSE, dash_V.m);
	uniforms++;
	
	smashed_sp = link_programme_from_files (SMASHED_VS, SMASHED_FS);
	
	tacho_sp = link_programme_from_files (TACHO_VS, TACHO_FS);
	tacho_P_loc = glGetUniformLocation (tacho_sp, "P");
	tacho_V_loc = glGetUniformLocation (tacho_sp, "V");
	tacho_M_loc = glGetUniformLocation (tacho_sp, "M");
	tacho_rpm_fac_loc = glGetUniformLocation (tacho_sp, "rpm_fac");
	GLint tacho_full_dm_loc = glGetUniformLocation (tacho_sp, "full_dm");
	GLint tacho_empty_dm_loc = glGetUniformLocation (tacho_sp, "empty_dm");
	glUseProgram (tacho_sp);
	glUniformMatrix4fv (tacho_V_loc, 1, GL_FALSE, dash_V.m);
	glUniform1i (tacho_full_dm_loc, 0);
	glUniform1i (tacho_empty_dm_loc, 1);
	uniforms++;
	
	gear_sp = link_programme_from_files (GEAR_VS, GEAR_FS);
	gear_P_loc = glGetUniformLocation (gear_sp, "P");
	gear_V_loc = glGetUniformLocation (gear_sp, "V");
	gear_M_loc = glGetUniformLocation (gear_sp, "M");
	glUseProgram (gear_sp);
	glUniformMatrix4fv (gear_V_loc, 1, GL_FALSE, dash_V.m);
	uniforms++;
	
	speedo_sp = link_programme_from_files (SPEEDO_VS, SPEEDO_FS);
	speedo_P_loc = glGetUniformLocation (speedo_sp, "P");
	speedo_V_loc = glGetUniformLocation (speedo_sp, "V");
	speedo_M_loc = glGetUniformLocation (speedo_sp, "M");
	glUseProgram (speedo_sp);
	glUniformMatrix4fv (speedo_V_loc, 1, GL_FALSE, dash_V.m);
	uniforms++;
	
	needle_sp = link_programme_from_files (NEEDLE_VS, NEEDLE_FS);
	needle_P_loc = glGetUniformLocation (needle_sp, "P");
	needle_V_loc = glGetUniformLocation (needle_sp, "V");
	needle_M_loc = glGetUniformLocation (needle_sp, "M");
	glUseProgram (needle_sp);
	glUniformMatrix4fv (needle_V_loc, 1, GL_FALSE, dash_V.m);
	uniforms++;
	
	// textures
	steering_diff_map = create_texture_from_file (STEERING_DIFF);
	smashed_diff_map = create_texture_from_file (SMASHED_DIFF);
	tacho_full_diff_map = create_texture_from_file (TACHO_FULL_DIFF);
	tacho_empty_diff_map = create_texture_from_file (TACHO_EMPTY_DIFF);
	speedo_diff_map = create_texture_from_file (SPEEDO_DIFF);
	{
		int i;
		
		char gear_ims[6][32] = {
			"textures/n.png",
			"textures/1st.png",
			"textures/2nd.png",
			"textures/3rd.png",
			"textures/4th.png",
			"textures/5th.png"
		};
		
		for (i = 0; i < 6; i++) {
			gear_dms[i] = create_texture_from_file (gear_ims[i]);
		}
	}
	
	dash_M = translate (identity_mat4 (), vec3 (0.0f, 0.0f, -1.125f));
	tacho_M = scale (identity_mat4 (), vec3 (0.15f, 0.1f, 1.0f));
	tacho_M = translate (tacho_M, vec3 (-0.6f, -0.4f, -1.125f));
	gear_M = scale (identity_mat4 (), vec3 (0.05f, 0.05f, 1.0f));
	gear_M = translate (gear_M, vec3 (0.6f, -0.5f, -1.125f));
	mirror_M = scale (identity_mat4 (), vec3 (0.4f, 0.4f, 0.4f));
	mirror_M = translate (mirror_M, vec3 (0.65f, 0.6f, -1.125f));
	speedo_M = scale (identity_mat4 (), vec3 (0.1125f, 0.11f, 0.15f));
	speedo_M = translate (speedo_M, vec3 (0.0f, -0.535f, -1.3f));
	needle_M = scale (identity_mat4 (), vec3 (0.1125f, 0.1f, 0.15f));
	needle_M = translate (needle_M, vec3 (0.0f, -0.535f, -1.3f));
	
	mirror_outer_M = scale (identity_mat4 (), vec3 (0.45f, 0.45f, 0.45f));
	mirror_outer_M = translate (mirror_outer_M, vec3 (0.65f, 0.6f, -1.125f));
	
	return true;
}

void set_rpm_fac (float fac) {
	glUseProgram (tacho_sp);
	glUniform1f (tacho_rpm_fac_loc, fac);
}

void set_steering (float deg) {
	mat4 R;
	
	steering_deg = deg;
	R = rotate_z_deg (identity_mat4 (), steering_deg);
	steering_M = translate (R, vec3 (0.0f, -0.7f, -1.125f));
}

void move_dash (vec3 p) {
	dash_pos = p;
}

void draw_dash () {
	if (draw_smashed) {
		// smashed!
		// I turn depth-write (masking) off so the other stuff draw over it
		glDisable (GL_DEPTH_TEST); // always draw, regarless of other stuff
		glDepthMask (GL_FALSE); // don't write to depth-buffer
		glUseProgram (smashed_sp);
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, smashed_diff_map);
		glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
		draws++;
		verts += 4;
		glDepthMask (GL_TRUE);
		glEnable (GL_DEPTH_TEST);
	}
	glUseProgram (dash_sp);
	if (cam_P_dirty) {
		P_boring = perspective (
			67.0f, (float)gl_width / (float)gl_height, 0.1f, 200.0f);
		glUniformMatrix4fv (dash_P_loc, 1, GL_FALSE, P_boring.m);
		glUniform1f (dash_h_loc, (float)gl_height);
		glUniform1f (dash_w_loc, (float)gl_width);
		uniforms += 3;
	}
	
	glUniformMatrix4fv (dash_M_loc, 1, GL_FALSE, dash_M.m);
	uniforms++;
	glBindVertexArray (dash_vao);
	glDrawArrays (GL_TRIANGLES, 0, dash_point_count);
	draws++;
	verts += dash_point_count;
	
	//
	// speedo
	glUseProgram (speedo_sp);
	if (cam_P_dirty) {
		glUniformMatrix4fv (speedo_P_loc, 1, GL_FALSE, P_boring.m);
		uniforms++;
	}
	glUniformMatrix4fv (speedo_M_loc, 1, GL_FALSE, speedo_M.m);
	uniforms++;
	glBindVertexArray (speedo_vao);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, speedo_diff_map);
	glDrawArrays (GL_TRIANGLES, 0, speedo_point_count);
	draws++;
	verts += speedo_point_count;
	
	//
	// speedo
	{
		float needle_deg;
		
		glUseProgram (needle_sp);
		if (cam_P_dirty) {
			glUniformMatrix4fv (needle_P_loc, 1, GL_FALSE, P_boring.m);
			uniforms++;
		}
		needle_deg = 90.0f - (
			(get_curr_speed ()* (1.0f / 0.27f)) / 300.0f) * 180.0f;
		needle_M = rotate_z_deg (identity_mat4 (), needle_deg);
		needle_M = scale (needle_M , vec3 (0.1125f, 0.1f, 0.15f));
		needle_M = translate (needle_M, vec3 (0.0f, -0.535f, -1.3f));
		glUniformMatrix4fv (needle_M_loc, 1, GL_FALSE, needle_M.m);
		uniforms++;
		glBindVertexArray (needle_vao);
		glDrawArrays (GL_TRIANGLES, 0, needle_point_count);
		draws++;
		verts += needle_point_count;
	}
	
	//
	// tachometer
	glUseProgram (tacho_sp);
	if (cam_P_dirty) {
		glUniformMatrix4fv (tacho_P_loc, 1, GL_FALSE, P_boring.m);
		uniforms++;
	}
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, tacho_full_diff_map);
	glActiveTexture (GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_2D, tacho_empty_diff_map);
	glUniformMatrix4fv (tacho_M_loc, 1, GL_FALSE, tacho_M.m);
	uniforms++;
	glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
	draws++;
	verts += 4;
	
	//
	// gear indicator
	glUseProgram (gear_sp);
	if (cam_P_dirty) {
		glUniformMatrix4fv (gear_P_loc, 1, GL_FALSE, P_boring.m);
		uniforms++;
	}
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, gear_dms[get_curr_gear ()]);
	glUniformMatrix4fv (gear_M_loc, 1, GL_FALSE, gear_M.m);
	uniforms++;
	glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
	draws++;
	verts += 4;
	
	glUseProgram (steering_sp);
	if (cam_P_dirty) {
		glUniformMatrix4fv (steering_P_loc, 1, GL_FALSE, P_boring.m);
		glUniform1f (steering_h_loc, (float)gl_height);
		glUniform1f (steering_w_loc, (float)gl_width);
		uniforms += 3;
	}
	
	//
	// steering wheel
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, steering_diff_map);
	glUniformMatrix4fv (steering_M_loc, 1, GL_FALSE, steering_M.m);
	uniforms++;
	glBindVertexArray (steering_vao);
	glDrawArrays (GL_TRIANGLES, 0, steering_point_count);
	draws++;
	verts += steering_point_count;
	
	//
	// mirror
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, cam_mirror_tex);
	// outside black bit of mirror
	glUseProgram (mirror_outer_sp);
	if (cam_P_dirty) {
		glUniformMatrix4fv (mirror_outer_P_loc, 1, GL_FALSE, P_boring.m);
		uniforms++;
	}
	glUniformMatrix4fv (mirror_outer_M_loc, 1, GL_FALSE, mirror_outer_M.m);
	uniforms++;
	glBindVertexArray (mirror_vao);
	glDrawArrays (GL_TRIANGLES, 0, mirror_point_count);
	draws++;
	verts += mirror_point_count;
	
	glUseProgram (mirror_sp);
	if (cam_P_dirty) {
		glUniformMatrix4fv (mirror_P_loc, 1, GL_FALSE, P_boring.m);
		uniforms++;
	}
	
	glUniformMatrix4fv (mirror_M_loc, 1, GL_FALSE, mirror_M.m);
	uniforms++;
	glBindVertexArray (mirror_vao);
	glDrawArrays (GL_TRIANGLES, 0, mirror_point_count);
	draws++;
	verts += mirror_point_count;
}
