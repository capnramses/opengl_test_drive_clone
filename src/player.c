#include "player.h"
#include "camera.h"
#include "gl_utils.h"
#include "maths_funcs.h"
#include "dash.h"

#define NUM_GEARS 4

float curr_heading = 0.0f;
vec3 curr_pos = vec3 (0.0f, 1.0f, 0.0f);
float curr_speed = 0.0f;
float turn_speed = 20.0f;
float gear_accel[NUM_GEARS] = {
	0.0f,
	2.0f,
	5.0f,
	10.0f
};
int curr_gear_no = 0;
// check if a key is still held down
bool was_key_down[1024];

float friction = 0.01f;

void update_player (double elapsed) {
	// W and S are accelerate and brake
	// A and D are the steering wheel
	// Q E are gear up/down
	if (GLFW_PRESS == glfwGetKey (gl_window, 'W')) {
		curr_speed += gear_accel[curr_gear_no] * (float)elapsed;
	}
	if (GLFW_PRESS == glfwGetKey (gl_window, 'S')) {
		curr_speed -= gear_accel[curr_gear_no] * (float)elapsed;
	}
	if (GLFW_PRESS == glfwGetKey (gl_window, 'A')) {
		curr_heading += turn_speed * (float)elapsed;
	} else if (GLFW_PRESS == glfwGetKey (gl_window, 'D')) {
		curr_heading -= turn_speed * (float)elapsed;
	// when user lets go of steering controls it centres
	} else {
		curr_heading = 0.0f;
	}
	if (GLFW_PRESS == glfwGetKey (gl_window, 'Q')) {
		if (!was_key_down[(int)'Q']) {
			curr_gear_no = curr_gear_no + 1 < NUM_GEARS ? curr_gear_no + 1 : curr_gear_no;
			printf ("changed gear to %i\n", curr_gear_no);
			was_key_down[(int)'Q'] = true;
		}
	}
	if (GLFW_PRESS == glfwGetKey (gl_window, 'E')) {
		if (!was_key_down[(int)'E']) {
			curr_gear_no = curr_gear_no - 1 >= 0 ? curr_gear_no - 1 : curr_gear_no;
			printf ("changed gear to %i\n", curr_gear_no);
			was_key_down[(int)'E'] = true;
		}
	}
	if (GLFW_RELEASE == glfwGetKey (gl_window, 'Q')) {
		was_key_down[(int)'Q'] = false;
	}
	if (GLFW_RELEASE == glfwGetKey (gl_window, 'E')) {
		was_key_down[(int)'E'] = false;
	}
	
	// calculate friction so that car slows down if you're not accel or braking
	float fric_deccel = friction * curr_speed;
	curr_speed -= fric_deccel;
	vec4 velocity = rotate_y_deg (identity_mat4 (), curr_heading) *
		vec4 (0.0f, 0.0f, -curr_speed * (float)elapsed, 0.0f);
	
	// update position
	curr_pos = curr_pos + vec3 (velocity);
	
	// update dashboard/car interior
	move_dash (curr_pos);
	
	// update cam
	/*vec3 targ = curr_pos + vec3 (
		rotate_y_deg (identity_mat4 (), curr_heading) *
		vec4 (0.0f, 0.0f, -1.0, 0.0f)
	);*/
	vec3 targ = curr_pos + vec3 (0.0f, 0.0f, -1.0);
	V = look_at (curr_pos, targ, vec3 (0.0f, 1.0f, 0.0f));
	cam_V_dirty = true;
}

