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
int time_text;
int fps_text;

int main (int argc, char** argv) {
	double prev_time = 0.0;
	double accum_sim_time = 0.0;
	int param = -1;

	myargc = argc;
	myargv = argv;
	gl_width = 800;
	gl_height = 800;

	param = M_CheckParm ("-help");
	if (param > 0) {
		printf ("ANTON'S TEST DRIVE REMAKE. PARAMS:\n"
			"-help\t\t\tthis\n"
			"-res WIDTH HEIGHT\tset viewport resolution\n"
			"-fs\t\t\tfull-scren\n"
			"-vidrec\t\t\trecord a video\n"
		);
		return 0;
	}
	param = M_CheckParm ("-res");
	if (param > 0) {
		gl_width = atoi (argv[param + 1]);
		gl_height = atoi (argv[param + 2]);
		printf ("===resoltion set to %iX%i===\n", gl_width, gl_height);
	}
	param = M_CheckParm ("-fs");
	if (param > 0) {
		printf ("===display set to full-screen===\n");
		full_screen = true;
	}

	if (!start_gl (gl_width, gl_height)) {
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
		-200.0f / (float)gl_width,
		1.0f,
		110.0f,
		0.9f,
		0.9f,
		0.0f,
		0.8f
	);
#ifdef DEBUG
	fps_text = add_text (
		"hz:\ndraws:\nuniforms:\nverts:",
		-1.0,
		0.0f,
		60.0f,
		0.9f,
		0.9f,
		0.0f,
		0.8f
	);
#endif
	init_player ();

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
	double fps_timer = 0.0;
	prev_time = glfwGetTime ();
	while (!glfwWindowShouldClose (gl_window)) {
		// work out how much time has passed
		double curr_time = glfwGetTime ();
		double elapsed = curr_time - prev_time;
		prev_time = curr_time;
		accum_sim_time += elapsed;
#ifdef DEBUG
		fps_timer += elapsed;
#endif
		draws = uniforms = verts = 0;
		
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
		
		if (finished_level ()) {
				char tmp[32];
				int mins, secs, ms;
				double dms;
				
				mins = (int)(curr_time / 60.0);
				secs = (int)(curr_time - (double)mins * 60.0);
				dms = curr_time - (double)mins * 60.0 - (double)secs;
				ms = (int)(dms * 100.0);
				
				sprintf (tmp, "%02i:%02i:%02i", mins, secs, ms); 
				printf ("Finished Level! Time: %s\n", tmp);
				printf ("Top speed reached: %i kph\n", (int)top_speed_reached);
				printf ("Crash count: %i\n", crash_count);
				
				break;
			}
		
		// don't update too often that it slows us down
		time_since_text_up += elapsed;
		if (time_since_text_up >= 0.05) {
			int mins, secs, ms;
			double dms;
			char tmp[32];
			
			mins = (int)(curr_time / 60.0);
			secs = (int)(curr_time - (double)mins * 60.0);
			dms = curr_time - (double)mins * 60.0 - (double)secs;
			ms = (int)(dms * 100.0);
			
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
		if (GLFW_PRESS == glfwGetKey (gl_window, GLFW_KEY_F11)) {
			assert (screenshot ());
		}
		if (GLFW_PRESS == glfwGetKey (gl_window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (gl_window, 1);
		}
		
#ifdef DEBUG
		frames++;
		if (fps_timer > 0.1) {
			char tmp[32];
			double hz = 0.0;
			
			hz = (double)frames / fps_timer;
			sprintf (tmp, "hz %.2lf:\ndraws %i:\nuniforms %i:\nverts: %i",
				hz, draws, uniforms, verts); 
			update_text (fps_text, tmp);
			
			fps_timer = 0.0;
			frames = 0;
		}
#endif
		
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
