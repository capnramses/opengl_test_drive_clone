#include "gl_utils.h"
#include "maths_funcs.h"
#include "camera.h"
#include "terrain.h"
#include "dash.h"
#include "player.h"
#include "traffic.h"
#include "audio.h"
#include "text.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define TIME_STEP_SIZE 0.02 // 50 Hz

bool dump_video;
// motion blur effect
bool enable_blur = false;
int blur_round;
int time_text;

int main (int argc, char** argv) {
	double prev_time = 0.0;
	double accum_sim_time = 0.0;

	myargc = argc;
	myargv = argv;

	if (!start_gl (800, 800)) {
		fprintf (stderr, "ERROR: could not start opengl\n");
		return 1;
	}
	
	// check for video record mode
	if (M_CheckParm ("-vidrec")) {
		printf ("===video recording enabled===\n");
		dump_video = true;
		reserve_video_memory ();
	}

	if (GL_TRUE == glfwJoystickPresent (GLFW_JOYSTICK_1)) {
		printf ("Joystick 0 [%s] detected\n",
			glfwGetJoystickName (GLFW_JOYSTICK_1));
		det_joystick = true;
	}

	init_audio ();
	init_cam ();
	init_terrain ();
	init_dash ();
	init_traffic ();
	assert (init_text_rendering (
		"fonts/freemono.png",
		"fonts/freemono.meta",
		gl_width,
		gl_height
	));
	time_text = add_text (
		"00:00:00",
		-0.21f,
		1.0f,
		110.0f,
		0.7f,
		0.7f,
		0.7f,
		0.8f
	);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	glClearColor (0.0, 0.5, 0.5, 1.0);
	glEnable (GL_CULL_FACE);
	glCullFace (GL_BACK);
	glFrontFace (GL_CCW);

	double video_timer = 0.0; // time video has been recording
	double video_dump_timer = 0.0; // timer for next frame grab
	double frame_time = 1.0 / (double)g_video_fps; // 1/25 seconds of time
	double time_since_text_up = 0.0;
	prev_time = glfwGetTime ();
	while (!glfwWindowShouldClose (gl_window)) {
		// work out how much time has passed
		double curr_time = glfwGetTime ();
		double elapsed = curr_time - prev_time;
		prev_time = curr_time;
		accum_sim_time += elapsed;
		
		if (dump_video) {
			video_timer += elapsed;
			video_dump_timer += elapsed;
			// only record 10s of video, then quit
			if (video_timer > (double)g_video_seconds_total) {
				printf ("video timer is %lf / %lf\n",
					video_timer, (double)g_video_seconds_total);
				break;
			}
		}
		
		// work out simulation time steps
		while (accum_sim_time > TIME_STEP_SIZE) {
			accum_sim_time -= TIME_STEP_SIZE;
			
			update_player (TIME_STEP_SIZE);
			update_traffic (TIME_STEP_SIZE);
		}
		
		// don't update too often that it slows us down
		time_since_text_up += elapsed;
		if (time_since_text_up >= 0.05) {
			int mins = (int)(curr_time / 60.0);
			int secs = (int)(curr_time - (double)mins * 60.0);
			double dms = curr_time - (double)mins * 60.0 - (double)secs;
			int ms = (int)(dms * 100.0);
			char tmp[32];
			sprintf (tmp, "%02i:%02i:%02i", mins, secs, ms);
			update_text (time_text, tmp);
			time_since_text_up = 0.0;
		}
		
		//
		// draw scene for rear-vision mirror
		// this renders to a texture
		switch_to_rear_view ();
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport (0, 0, CAM_TEX_WIDTH, CAM_TEX_HEIGHT);
		draw_terrain ();
		draw_traffic ();
		
		//
		// draw normal scene
		switch_to_front_view ();
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport (0, 0, gl_width, gl_height);
		draw_terrain ();
		draw_traffic ();
		
		// i dont want the dashboard to ever intersect with background so
		// i do a clear of the depth buffer
		glClear (GL_DEPTH_BUFFER_BIT);
		draw_dash ();
		draw_texts ();
		
		if (dump_video) { // check if recording mode is enabled
			while (video_dump_timer > frame_time) {
				grab_video_frame (); // 25 Hz so grab a frame
				video_dump_timer -= frame_time;
			}
		}
		
		// can expect everything has updated camera matrices by now
		cam_P_dirty = false;
		cam_V_dirty = false;
		
		// screenshot on F11
		if (glfwGetKey (gl_window, GLFW_KEY_F11)) {
			assert (screenshot ());
		}

		glfwPollEvents ();
		glfwSwapBuffers (gl_window);
	}
	
	if (dump_video) {
		dump_video_frames ();
	}

	glfwTerminate ();
	free_audio ();
	return 0;
}
