#include "gl_utils.h"
#include "maths_funcs.h"
#include "obj_parser.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "terrain.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define SWORD_VS "shaders/test.vert"
#define SWORD_FS "shaders/test.frag"

int main () {
	GLuint sp;
	GLuint vao;
	int point_count = 0;

	if (!start_gl (800, 800)) {
		fprintf (stderr, "ERROR: could not start opengl\n");
		return 1;
	}

	init_terrain ();

	sp = link_programme_from_files (SWORD_VS, SWORD_FS);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	glClearColor (0.01, 0.01, 0.25, 1.0);

	while (!glfwWindowShouldClose (gl_window)) {
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport (0, 0, gl_width, gl_height);
		
		glUseProgram (sp);
		//glBindVertexArray (vao);
		//glDrawArrays (GL_TRIANGLES, 0, point_count);

		glfwPollEvents ();
		glfwSwapBuffers (gl_window);
	}

	return 0;
}
