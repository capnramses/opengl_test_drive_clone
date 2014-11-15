#include "player.h"
#include "camera.h"
#include "gl_utils.h"
#include "maths_funcs.h"
#include "dash.h"
#include "audio.h"
#include "traffic.h"
#include "terrain.h"

/* TODO

gear ratio should be
multiply speed, divide acceleration
*/

#define NUM_GEARS 4

float curr_heading = 0.0f;
vec3 curr_pos = vec3 (1.0f, 0.5f, 0.0f);
float curr_speed = 0.0f;
float turn_speed = 50.0f;
float gear_accel[NUM_GEARS] = {
	0.0f,
	2.5f,
	5.0f,
	10.0f
};
int curr_gear_no = 0;
// check if a key is still held down
bool was_key_down[1024];
bool was_button_down[64];
bool det_joystick;
float friction = 0.01f;

//
// win detection based on z dist
// returns true if finished
bool finished_level () {
	if (curr_pos.v[2] < -792.0f) {
		return true;
	}
	return false;
}

void update_player (double elapsed) {
	float wheel_turn = 0.0f;

	if (det_joystick) {
		const float* axis_values = NULL;
		const unsigned char* buttons = NULL;
		float pedal_factor = 0.0f;
		float brake_factor = 0.0f;
		float turn_factor = 0.0f;
		int count = 0;
		//int i;
		
		axis_values = glfwGetJoystickAxes (GLFW_JOYSTICK_1, &count);
		//for (i = 0; i < count; i++) {
		//	printf ("axis %i) %f\n", i, axis_values[i]);
		//}
		
		count = 0;
		buttons = glfwGetJoystickButtons (GLFW_JOYSTICK_1, &count);
		//for (i = 0; i < count; i++) {
		//	printf ("button %i) %i\n", i, buttons[i]);
		//}
		
		// gear up
		if (buttons[5]) {
			if (!was_button_down[5]) {
				curr_gear_no =
				curr_gear_no + 1 < NUM_GEARS ? curr_gear_no + 1 : curr_gear_no;
				printf ("changed gear to %i\n", curr_gear_no);
				was_button_down[5] = true;
			}
		} else {
			was_button_down[5] = false;
		}
		if (buttons[4]) {
			if (!was_button_down[4]) {
				curr_gear_no =
				curr_gear_no = curr_gear_no - 1 >= 0 ? curr_gear_no - 1 : curr_gear_no;
				printf ("changed gear to %i\n", curr_gear_no);
				was_button_down[4] = true;
			}
		} else {
			was_button_down[4] = false;
		}
		
		// accel
		// axis[5] -1 = full pedal down, 1 = released
		pedal_factor = axis_values[5] * -0.5f + 0.5f;
		curr_speed += gear_accel[curr_gear_no] * pedal_factor * (float)elapsed;
		brake_factor = axis_values[2] * 0.5f + 0.5f;
		//printf ("bf %f\n", brake_factor);
		curr_speed -= 15.0f * brake_factor * (float)elapsed;
		if (curr_speed < 0.0f) {
			curr_speed = 0.0f;
		}
		
		// steering axis 3. 1=left -1=right
		if (curr_speed > 1.0f) {
			turn_factor = axis_values[3] * 1.5f;
			// get rid of noise
			turn_factor = (float)((int)(turn_factor * 10.0f)) / 10.0f;
			curr_heading += turn_speed * turn_factor * (float)elapsed;
			wheel_turn += turn_speed * turn_factor * (float)elapsed;
		}
	}
	// W and S are accelerate and brake
	// A and D are the steering wheel
	// Q E are gear up/down
	if (GLFW_PRESS == glfwGetKey (gl_window, 'W')) {
		curr_speed += gear_accel[curr_gear_no] * (float)elapsed;
	}
	if (GLFW_PRESS == glfwGetKey (gl_window, 'S')) {
		curr_speed -= 15.0f * (float)elapsed;
		if (curr_speed < 0.0f) {
			curr_speed = 0.0f;
		}
	}
	if (curr_speed > 1.0f) {
		if (GLFW_PRESS == glfwGetKey (gl_window, 'A')) {
			curr_heading += turn_speed * (float)elapsed;
			wheel_turn += turn_speed * (float)elapsed;
		} else if (GLFW_PRESS == glfwGetKey (gl_window, 'D')) {
			curr_heading -= turn_speed * (float)elapsed;
			wheel_turn -= turn_speed * (float)elapsed;
		// when user lets go of steering controls it centres
		}
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
	set_steering (wheel_turn * 50.0f);
	
	// update cam
	/*vec3 targ = curr_pos + vec3 (
		rotate_y_deg (identity_mat4 (), curr_heading) *
		vec4 (0.0f, 0.0f, -1.0, 0.0f)
	);*/
	set_heading (curr_heading);
	move_cam (curr_pos);
	
	
	//printf ("s %.2f\n", curr_speed);
	set_engine_speed (curr_speed + 1.0f);
	// also change camera perspective as speed changes
	float fovrange = 150.0f - 67.0f;
	float fovfac =  fabs(curr_speed) / 30.0f;
	float fovnew = 67.0f + fovrange * fovfac;
	set_fovy (fovnew);
	
	//
	// crash detection with other vehicles
	if (hit_truck (curr_pos)) {
		printf ("we hit a truck!\n");
		play_crash_snd ();
	}
	//
	// crash detection with terrain
	if (hit_wall (curr_pos)) {
		//printf ("we hit terrain!\n");
		play_crash_snd ();
	}
}

