//
// Simple OpenGL Text Rendering Functions
// Dr Anton Gerdelan, Trinity College Dublin, Ireland
// First version 4 Nov 2014
// =Pros=
// * will just magically work
// * easy-ish to change the font
// =Cons=
// * i didn't add support to render multiple fonts at once (but trivial to add)
// =Dependencies=
// Sean Barrett's public domain image loader lib: stb_image.h (included)
// =Possible Easy Extensions=
// * support multiple fonts
// * add a flag to allow text to be invisible/visible
// * rotate text
// * place text in 3d
//

#ifndef _TEXT_H_
#define _TEXT_H_

// change or comment these out if using different OpenGL libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//
// call before main loop to load shaders and stuff
bool init_text_rendering (
	const char* font_image_file,
	const char* font_meta_data_file,
	int viewport_width,
	int viewport_height
);

//
// add a string of text to render on-screen
// returns an integer to identify it with later if we want to change the text
// returns <0 on error
// x,y are position of the bottom-left of the first character in clip space
// size_is_px is the size of maximum-sized glyph in pixels on screen
// r, g, b, a is the colour of the text string
int add_text (
	const char* str,
	float x,
	float y,
	float size_in_px,
	float r,
	float g,
	float b,
	float a
);

//
// change text string in any previously added text
bool update_text (int id, const char* str);

//
// change colour of any previously added text
bool change_text_colour (int id, float r, float g, float b, float a);

//
// draw all the texts to screen
void draw_texts ();

#endif
