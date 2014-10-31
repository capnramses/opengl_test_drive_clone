#include "post.h"
#include "gl_utils.h"
#include <stdio.h>
#include <assert.h>

#define QUAD_VS "shaders/quad.vert"
#define QUAD_FS "shaders/quad.frag"

GLuint post_fb[NUM_BLUR_TEX];
GLuint post_fb_tex[NUM_BLUR_TEX];
GLuint quad_sp;
GLint quad_tex0_loc, quad_tex1_loc;
GLuint quad_vao;

bool init_post () {
	int i;
	float points[] = {
		-1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, -1.0f
	};
	GLuint vp_vbo;
	
	glGenFramebuffers (NUM_BLUR_TEX, post_fb);
	glGenTextures (NUM_BLUR_TEX, post_fb_tex);
	
	for (i = 0; i < NUM_BLUR_TEX; i++) {
		GLuint depth_tex;
	
		glBindFramebuffer (GL_FRAMEBUFFER, post_fb[i]);
		// create depth texture (enabled depth-sorting in a secondary framebuffer)
		glGenTextures (1, &depth_tex);
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, depth_tex);
		glTexImage2D (
			GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT,
			gl_width,
			gl_height,
			0,
			GL_DEPTH_COMPONENT,
			GL_UNSIGNED_BYTE,
			NULL
		);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach depth texture to framebuffer
		glFramebufferTexture2D (
			GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
		// create texture
	
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, post_fb_tex[i]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// make the texture the same size as the viewport
		glTexImage2D (
			GL_TEXTURE_2D,
			0,
			GL_SRGB_ALPHA,
			gl_width,
			gl_height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			NULL
		);
		// attach texture to fb
		glFramebufferTexture2D (
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, post_fb_tex[i], 0);
		// redirect fragment shader output 0 used to the texture that we just bound
		GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers (1, draw_bufs);
	
		// check for completeness
		assert (verify_bound_framebuffer ());
	}
	// bind the default fb back
	glBindFramebuffer (GL_FRAMEBUFFER, 0);

	// quad-shaders
	quad_sp = link_programme_from_files (QUAD_VS, QUAD_FS);
	quad_tex0_loc = glGetUniformLocation (quad_sp, "tex0");
	quad_tex1_loc = glGetUniformLocation (quad_sp, "tex1");
	glUseProgram (quad_sp);
	glUniform1i (quad_tex0_loc, 0);
	glUniform1i (quad_tex0_loc, 1);
	
	// VBO for fullscreen quad
	glGenBuffers (1, &vp_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, sizeof (points), points, GL_STATIC_DRAW);
	glGenVertexArrays (1, &quad_vao);
	glBindVertexArray (quad_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	
	return true;
}

void draw_blur () {
	int i;
	glUseProgram (quad_sp);
	for (i = 0; i < NUM_BLUR_TEX; i++) {
		glActiveTexture (GL_TEXTURE0 + i);
		glBindTexture (GL_TEXTURE_2D, post_fb_tex[i]);
	}
	glBindVertexArray (quad_vao);
	glDrawArrays (GL_TRIANGLES, 0, 6);
}
