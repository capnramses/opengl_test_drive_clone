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
#define MAX_TRUCKS 128
#define MAX_LANE_MARKERS 512

Vehicle trucks[MAX_TRUCKS];
vec3 right_lane_markers[MAX_LANE_MARKERS], left_lane_markers[MAX_LANE_MARKERS];
GLuint truck_vao, truck_wheel_vao;
int truck_point_count, truck_wheel_point_count;
int num_trucks;
int num_right_lane_markers, num_left_lane_markers;
GLuint truck_sp;
GLint truck_M_loc, truck_V_loc, truck_P_loc;
GLuint truck_diff_map, truck_wheel_diff_map;
float wheel_rot_speed = 500.0f;
float curr_wheel_rot = 0.0f;
float truck_drive_speed = 3.5f;

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
	
	add_truck_left_lane (0);
	add_truck_left_lane (1);
	add_truck_left_lane (2);
	add_truck_left_lane (3);
	add_truck_left_lane (5);
	add_truck_left_lane (8);
	add_truck_left_lane (13);
	add_truck_left_lane (20);
	add_truck_left_lane (30);
	add_truck_left_lane (35);
	add_truck_left_lane (50);
	add_truck_left_lane (60);
	add_truck_left_lane (90);
	add_truck_left_lane (110);
	
	add_truck_right_lane (4);
	add_truck_right_lane (8);
	add_truck_right_lane (20);
	add_truck_right_lane (25);
	add_truck_right_lane (35);
	add_truck_right_lane (48);
	add_truck_right_lane (55);
	add_truck_right_lane (60);
	add_truck_right_lane (65);
	add_truck_right_lane (67);
	add_truck_right_lane (69);
	add_truck_right_lane (70);
	add_truck_right_lane (130);
	add_truck_right_lane (150);
	
	return true;
}

bool add_truck_left_lane (int marker_num) {
	assert (num_trucks < MAX_TRUCKS);
	assert (marker_num < num_left_lane_markers);
	trucks[num_trucks].pos = left_lane_markers[marker_num];
	trucks[num_trucks].in_left_lane = true;
	trucks[num_trucks].curr_lane_marker = marker_num;
	num_trucks++;
	return true;
}


bool add_truck_right_lane (int marker_num) {
	assert (num_trucks < MAX_TRUCKS);
	assert (marker_num < num_right_lane_markers);
	trucks[num_trucks].pos = right_lane_markers[marker_num];
	trucks[num_trucks].in_left_lane = false;
	trucks[num_trucks].curr_lane_marker = marker_num;
	num_trucks++;
	return true;
}

bool update_traffic (double elapsed) {
	int i;
	vec3 curr_dest;
	
	// rotate truck wheels
	curr_wheel_rot += (float)elapsed * wheel_rot_speed;
	
	// update each truck's movement
	for (i = 0; i < num_trucks; i++) {
		// get distance to current road marker
		// if less than 1m, move onto next marker
		if (trucks[i].in_left_lane) {
			float dist;
			
			dist = length (trucks[i].pos -
				left_lane_markers[trucks[i].curr_lane_marker]);
			if (fabs (dist) < 1.0f) {
				trucks[i].curr_lane_marker--;
				if (trucks[i].curr_lane_marker < 0) {
					// loop around when get to end of road
					trucks[i].curr_lane_marker = num_left_lane_markers - 1;
					// teleport back to start
					trucks[i].pos = left_lane_markers[trucks[i].curr_lane_marker];
				} // endif node0
			} // endif dist
			curr_dest = left_lane_markers[trucks[i].curr_lane_marker];
		} else { // endif left lane
			float dist;
			
			dist = length (trucks[i].pos -
				right_lane_markers[trucks[i].curr_lane_marker]);
			if (fabs (dist) < 1.0f) {
				trucks[i].curr_lane_marker++;
				if (trucks[i].curr_lane_marker >= num_right_lane_markers) {
					// loop around when get to end of road
					trucks[i].curr_lane_marker = 0;
					// teleport back to start
					trucks[i].pos = right_lane_markers[trucks[i].curr_lane_marker];
				} // endif node0
			} // endif dist
			curr_dest = right_lane_markers[trucks[i].curr_lane_marker];
		}// endif right lane
			
		{ // move towards next marker
			trucks[i].fwd = normalise (curr_dest - trucks[i].pos);
			trucks[i].pos += trucks[i].fwd * truck_drive_speed * (float)elapsed;
		}
		
	} // endfor
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
		mat4 truck_M, R, T, wheel_M;
		vec3 rgt, up;
		
		// truck chassis
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, truck_diff_map);
		glBindVertexArray (truck_vao);
		// build a rotation matrix from fwd,right,up vectors to point truck in
		// direction it's heading in
		// this is just an inverse lookAt() - could have done inverse (look_at(..))
		up = vec3 (0.0f, 1.0f, 0.0f);
		rgt = cross (up, trucks[i].fwd);
		R.m[0] = rgt.v[0];
		R.m[1] = up.v[0];
		R.m[2] = -trucks[i].fwd.v[0];
		R.m[3] = 0.0f;
		R.m[4] = rgt.v[1];
		R.m[5] = up.v[1];
		R.m[6] = -trucks[i].fwd.v[1];
		R.m[7] = 0.0f;
		R.m[8] = -rgt.v[2];
		R.m[9] = -up.v[2];
		R.m[10] = trucks[i].fwd.v[2];
		R.m[11] = 0.0f;
		R.m[12] = 0.0f;
		R.m[13] = 0.0f;
		R.m[14] = 0.0f;
		R.m[15] = 1.0f;
		T = translate (identity_mat4 (), trucks[i].pos);
		truck_M = T * R;
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, truck_M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_point_count);
		
		// add 4 wheels
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, truck_wheel_diff_map);
		glBindVertexArray (truck_wheel_vao);
		wheel_M = rotate_x_deg (identity_mat4 (), curr_wheel_rot);
		wheel_M = translate (wheel_M, vec3 (-0.4f, 0.15, 0.45));
		// inherit rotation and translation of parent (the truck)
		wheel_M = truck_M * wheel_M;
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, wheel_M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_wheel_point_count);
		
		glBindVertexArray (truck_wheel_vao);
		wheel_M = rotate_x_deg (identity_mat4 (), curr_wheel_rot);
		wheel_M = translate (wheel_M, vec3 (0.4f, 0.15, 0.45));
		// inherit rotation and translation of parent (the truck)
		wheel_M = truck_M * wheel_M;
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, wheel_M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_wheel_point_count);
		
		glBindVertexArray (truck_wheel_vao);
		wheel_M = rotate_x_deg (identity_mat4 (), curr_wheel_rot);
		wheel_M = translate (wheel_M, vec3 (-0.45f, 0.15, -1.6));
		// inherit rotation and translation of parent (the truck)
		wheel_M = truck_M * wheel_M;
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, wheel_M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_wheel_point_count);
		
		glBindVertexArray (truck_wheel_vao);
		wheel_M = rotate_x_deg (identity_mat4 (), curr_wheel_rot);
		wheel_M = translate (wheel_M, vec3 (0.45f, 0.15, -1.6));
		// inherit rotation and translation of parent (the truck)
		wheel_M = truck_M * wheel_M;
		glUniformMatrix4fv (truck_M_loc, 1, GL_FALSE, wheel_M.m);
		glDrawArrays (GL_TRIANGLES, 0, truck_wheel_point_count);
	}
	
	return true;
}

bool add_left_lane_marker (vec3 pos) {
	assert (num_left_lane_markers < MAX_LANE_MARKERS);
	left_lane_markers[num_left_lane_markers] = pos;
	num_left_lane_markers++;
	return true;
}

bool add_right_lane_marker (vec3 pos) {
	assert (num_right_lane_markers < MAX_LANE_MARKERS);
	right_lane_markers[num_right_lane_markers] = pos;
	num_right_lane_markers++;
	return true;
}

//
// check if given position is too close to a truck
// returns true if too close
bool hit_truck (vec3 pos) {
	float too_close = 1.0f;
	int i;
	
	for (i = 0; i < num_trucks; i++) {
		vec3 truck_pos;
		float distsq;
		
		truck_pos = trucks[i].pos;
		// get squared distance to avoid expensive sqrt calculation
		distsq = length2 (truck_pos - pos);
		if (distsq < too_close * too_close) {
			return true;
		}
	}
	
	return false;
}

//
// detect collision with wall on RHS
// uses traffic lane markers to do so
// return true if crashed
bool hit_wall (vec3 pos) {
	vec3 ahead_pos, behind_pos;
	float wallx = 0.0f;
	float edgex = 0.0f;
	int ahead_marker, behind_marker;
	
	ahead_marker = 0;
	behind_marker = -1;
	
	{ // get road marker pos ahead and behind car
		float closest_distsq = 100000.0f;
		int i;
		
		for (i = 0; i < num_left_lane_markers; i++) {
			// only looking for marker AHEAD of current pos
			if (left_lane_markers[i].v[2] <= pos.v[2]) {
				float distsq;
				
				distsq = length2 (left_lane_markers[i] - pos);
				if (distsq < closest_distsq) {
					closest_distsq = distsq;
					ahead_marker = i;
				}
			}
		} // endfor
		behind_marker = ahead_marker - 1;
		
		//printf ("ahead marker %i\nbehind marker %i\n", ahead_marker, behind_marker);
	}
	
//	printf ("\n\n");
	
	// the 0.75 puts marker back in centre of road
	ahead_pos = left_lane_markers[ahead_marker] + vec3 (0.75f, 0.0f, 0.0f);
	if (behind_marker < 0) {
		behind_pos = vec3 (0.0f, 0.0f, 0.0f);
	} else {
		behind_pos = left_lane_markers[ahead_marker - 1] +
			vec3 (0.75f, 0.0f, 0.0f);
	}
	
	{ // work out wall xpos at specific location
		float xrange, zrange, xgrad, ourzfac, interpmarkerx;
		float wall_leeway = 0.0f; // make cliff and edge a bit farther
		float cliff_leeway = 0.0f;
		float ztotalbtw = 4.0f;
		
//		print (behind_pos);
//		print (ahead_pos);
	
		xrange = ahead_pos.v[0] - behind_pos.v[0];
		
		zrange = fabs (ahead_pos.v[2] - behind_pos.v[2]);
	
		xgrad = xrange / zrange;
//		printf ("xgrad = %f\n", xgrad);
	
		ourzfac = fabs (pos.v[2] - behind_pos.v[2]) / zrange;
//		printf ("our zpos %.2f\nour zfac =%.2f\n", pos.v[2], ourzfac);
		
		interpmarkerx = behind_pos.v[0] + xgrad * ourzfac * ztotalbtw;
		
//		printf ("interpx: %f\n", interpmarkerx);
		wallx = interpmarkerx + 2.0f + wall_leeway;
		edgex = interpmarkerx - (2.0f + cliff_leeway);
		
//	printf (" wallx %.2f\n edgex %.2f\n", wallx, edgex);
	}
//	printf ("==posx %.2f==\n", pos.v[0]);
	if (pos.v[0] > wallx) {
		printf ("HIT WALL on RHS\n");
		return true;
	}
	if (pos.v[0] < edgex) {
		printf ("FELL OFF CLIFF on LHS\n");
		return true;
	}
	
	return false;
}

vec3 get_closest_node_to (vec3 pos) {
	int i;
	float closest_distsq = 100000.0f;
	int closest = 0;

	for (i = 1; i < num_right_lane_markers; i++) {
		float distsq;
		
		distsq = length2 (right_lane_markers[i] - pos);
		if (distsq < closest_distsq) {
			closest_distsq = distsq;
			closest = i;
		}
	} // endfor
	
	return right_lane_markers[closest];
}

