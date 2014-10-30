#include "gl_utils.h"
#include "maths_funcs.h"
#include "camera.h"
#include "terrain.h"
#include "dash.h"
#include "player.h"
#include "traffic.h"

/*#include "obj_parser.h"
"*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define TIME_STEP_SIZE 0.02 // 50 Hz

bool dump_video;

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

	init_cam ();
	init_terrain ();
	init_dash ();
	init_traffic ();

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	glClearColor (0.0, 0.5, 0.5, 1.0);
	glEnable (GL_CULL_FACE);
	glCullFace (GL_BACK);
	glFrontFace (GL_CCW);

	double video_timer = 0.0; // time video has been recording
	double video_dump_timer = 0.0; // timer for next frame grab
	double frame_time = 0.04; // 1/25 seconds of time
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
			if (video_timer > 10.0) {
				break;
			}
		}
		
		// work out simulation time steps
		while (accum_sim_time > TIME_STEP_SIZE) {
			accum_sim_time -= TIME_STEP_SIZE;
			
			update_player (TIME_STEP_SIZE);
			update_traffic (TIME_STEP_SIZE);
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

		// can expect everything has updated camera matrices by now
		cam_P_dirty = false;
		cam_V_dirty = false;
		
		if (dump_video) { // check if recording mode is enabled
			while (video_dump_timer > frame_time) {
				grab_video_frame (); // 25 Hz so grab a frame
				video_dump_timer -= frame_time;
			}
		}

		glfwPollEvents ();
		glfwSwapBuffers (gl_window);
	}
	
	if (dump_video) {
		dump_video_frames ();
	}

	glfwTerminate ();
	return 0;
}
