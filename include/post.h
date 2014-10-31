#ifndef _POST_H_
#define _POST_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NUM_BLUR_TEX 2

bool init_post ();
void draw_blur ();

// secondary framebuffer
extern GLuint post_fb[NUM_BLUR_TEX];
extern GLuint post_fb_tex[NUM_BLUR_TEX];

#endif
