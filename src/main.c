#include "gl_utils.h"
#include "maths_funcs.h"
#include "camera.h"
#include "terrain.h"
#include "dash.h"
#include "player.h"

/*#include "obj_parser.h"
"*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define TIME_STEP_SIZE 0.02 // 50 Hz

//
// do one time-step of updates. this is a fixed real-time rate i.e.
// 10 steps per second, rather than graphics which render as fast as possible
// this allows consistent movements etc. that are not dependent on the frame
// rate
void time_step () {

}

int main () {
	double prev_time = 0.0;
	double accum_sim_time = 0.0;

	if (!start_gl (800, 800)) {
		fprintf (stderr, "ERROR: could not start opengl\n");
		return 1;
	}

	init_cam ();
	init_terrain ();
	init_dash ();

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	glClearColor (0.0, 0.5, 0.5, 1.0);

	prev_time = glfwGetTime ();
	while (!glfwWindowShouldClose (gl_window)) {
		// work out how much time has passed
		double curr_time = glfwGetTime ();
		double elapsed = curr_time - prev_time;
		prev_time = curr_time;
		accum_sim_time += elapsed;
		
		// work out simulation time steps
		while (accum_sim_time > TIME_STEP_SIZE) {
			accum_sim_time -= TIME_STEP_SIZE;
			
			update_player (TIME_STEP_SIZE);
		}
		
		
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport (0, 0, gl_width, gl_height);
		
		draw_terrain ();
		draw_dash ();

		// can expect everything has updated camera matrices by now
		cam_P_dirty = false;
		cam_V_dirty = false;

		glfwPollEvents ();
		glfwSwapBuffers (gl_window);
	}

	return 0;
}
