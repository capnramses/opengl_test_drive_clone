#include "player.h"
#include "camera.h"
#include "gl_utils.h"
#include "dash.h"
#include "audio.h"
#include "traffic.h"
#include "terrain.h"
#include "lambo.h"
#include "text.h"

// comment this line out for testing w/o crash detection
#define CRASHES

float curr_heading = 0.0f;
vec3 curr_pos = vec3 (1.0f, 0.5f, 0.0f);
float curr_speed = 0.0f;
float turn_speed = 50.0f;
double crash_count_down;
double immune_count_down;
// check if a key is still held down
bool was_key_down[1024];
bool was_button_down[64];
bool det_joystick;
float friction = 0.01f;
float brake_power = 30.0f;

int kph_text = -1;

vec3 get_player_pos () {
	return curr_pos;
}

//
// win detection based on z dist
// returns true if finished
bool finished_level () {
	if (curr_pos.v[2] < -792.0f) {
		return true;
	}
	return false;
}

//
// make some text
bool init_player () {
	kph_text = add_text (
		"0 RPM      0 KPH",
		-700.0f / (float)gl_width,
		-1.0f + 400.0f / (float)gl_height,
		110.0f,
		0.9f,
		0.9f,
		0.0f,
		0.8f
	);
	return true;
}

void update_player (double elapsed) {
	float wheel_turn = 0.0f;
	
	if (crash_count_down > 0.0) {
		crash_count_down -= elapsed;
		draw_smashed = true;
		if (crash_count_down <= 0.0) {
			immune_count_down = 1.5;
			// put player at closest node thing and reset speed etc.
			curr_heading = 0.0f;
			curr_speed = 0.0f;
			curr_gear = 0;
			curr_pos = get_closest_node_to (curr_pos) + vec3 (0.0, 0.5f, 0.0f);
			draw_smashed = false;
		}
		return;
	}
	if (immune_count_down > 0.0) {
		immune_count_down -= elapsed;
	}
	
	throttle_fac = 0.0f;
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
				if (ch_gear_up ()) {
					printf ("changed gear to %i\n", curr_gear);
				}
				was_button_down[5] = true;
			}
		} else {
			was_button_down[5] = false;
		}
		if (buttons[4]) {
			if (!was_button_down[4]) {
				if (ch_gear_down ()) {
					printf ("changed gear to %i\n", curr_gear);
				}
				was_button_down[4] = true;
			}
		} else {
			was_button_down[4] = false;
		}
		
		// accel
		// axis[5] -1 = full pedal down, 1 = released
		pedal_factor = axis_values[5] * -0.5f + 0.5f;
		brake_factor = axis_values[2] * 0.5f + 0.5f;
		//printf ("bf %f\n", brake_factor);
		curr_speed -= brake_power * brake_factor * (float)elapsed;
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
		throttle_fac = 1.0f;
	}
	if (GLFW_PRESS == glfwGetKey (gl_window, 'S')) {
		curr_speed -= brake_power * (float)elapsed;
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
			if (ch_gear_up ()) {
				printf ("changed gear to %i\n", curr_gear);
			}
			was_key_down[(int)'Q'] = true;
		}
	}
	if (GLFW_PRESS == glfwGetKey (gl_window, 'E')) {
		if (!was_key_down[(int)'E']) {
			if (ch_gear_down ()) {
				printf ("changed gear to %i\n", curr_gear);
			}
			was_key_down[(int)'E'] = true;
		}
	}
	if (GLFW_RELEASE == glfwGetKey (gl_window, 'Q')) {
		was_key_down[(int)'Q'] = false;
	}
	if (GLFW_RELEASE == glfwGetKey (gl_window, 'E')) {
		was_key_down[(int)'E'] = false;
	}
	
	// motor
	float accel = update_motor (curr_speed);
	curr_speed += accel * (float)elapsed;
	if (curr_speed < 0.0f) {
		curr_speed = 0.0f;
	}
	// modify because graphics are smaller than 1 unit per meter
	float kph = curr_speed * (1.0f / 0.27f);
	
	{
		char tmp[128];
		sprintf (tmp, "%i RPM   %i KPH   %i gear", (int)curr_motor_rpm,
			(int)kph, curr_gear);
		update_text (kph_text, tmp);
	}
	
	//printf ("kph %f accel %f\n", kph, accel);
	// calculate friction so that car slows down if you're not accel or braking
	//float fric_deccel = friction * curr_speed;
	//curr_speed -= fric_deccel;
	vec4 velocity = rotate_y_deg (identity_mat4 (), curr_heading) *
		vec4 (0.0f, 0.0f, -curr_speed * (float)elapsed, 0.0f);
	
	// update position
	// graphics fac
	float gfx = 0.75f;
	curr_pos = curr_pos + vec3 (velocity) * gfx;
	
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
	set_engine_speed (curr_motor_rpm * 0.002f + 0.002f);
	// also change camera perspective as speed changes
	float fovrange = 150.0f - 67.0f;
	float fovfac =  fabs(curr_speed * gfx) / 60.0f;
	float fovnew = 67.0f + fovrange * fovfac;
	set_fovy (fovnew);
	
#ifdef CRASHES
	if (immune_count_down <= 0.0) {
		//
		// crash detection with other vehicles
		if (hit_truck (curr_pos)) {
			printf ("we hit a truck!\n");
			play_crash_snd ();
			crash_count_down = 3.0;
			set_engine_speed (0.0f);
		}
		//
		// crash detection with terrain
		if (hit_wall (curr_pos)) {
			//printf ("we hit terrain!\n");
			play_crash_snd ();
			crash_count_down = 3.0;
			set_engine_speed (0.0f);
		}
	}
#endif
}

