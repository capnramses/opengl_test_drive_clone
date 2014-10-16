#include "gl_utils.h"
#include "maths_funcs.h"
#include "camera.h"
#include "terrain.h"

/*#include "obj_parser.h"
"*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int main () {
	if (!start_gl (800, 800)) {
		fprintf (stderr, "ERROR: could not start opengl\n");
		return 1;
	}

	init_cam ();
	init_terrain ();

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	glClearColor (0.0, 0.5, 0.5, 1.0);

	while (!glfwWindowShouldClose (gl_window)) {
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport (0, 0, gl_width, gl_height);
		
		draw_terrain ();

		// can expect everything has updated camera matrices by now
		cam_P_dirty = false;
		cam_V_dirty = false;

		glfwPollEvents ();
		glfwSwapBuffers (gl_window);
	}

	return 0;
}
