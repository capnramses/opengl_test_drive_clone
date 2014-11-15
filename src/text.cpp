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
//

#include "text.h"
#include "gl_utils.h"
#include "stb_image.h" // Sean Barrett's image loader
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// size of atlas. my handmade image is 16x16 glyphs
#define ATLAS_COLS 16
#define ATLAS_ROWS 16
#define MAX_STRINGS 256

struct Renderable_Text {
	GLuint points_vbo, texcoords_vbo;
	GLuint vao;
	float x, y;
	float size_px;
	float r, g, b, a;
	int point_count;
};

GLuint font_texture;
GLuint font_sp; // shader programme
GLint font_sp_text_colour_loc;
Renderable_Text renderable_texts[MAX_STRINGS];
float glyph_y_offsets[256] = { 0.0f };
float glyph_widths[256] = { 0.0f };
int font_viewport_width;
int font_viewport_height;
int num_render_strings;

bool load_font_meta (const char* meta_file) {
	char line [128];
	int ascii_code = -1;
	float prop_xMin = 0.0f;
	float prop_width = 0.0f;
	float prop_yMin = 0.0f;
	float prop_height = 0.0f;
	float prop_y_offset = 0.0f;
	FILE* fp = NULL;
	
	printf ("loading font meta-data from file: %s\n", meta_file);
	fp = fopen (meta_file, "r");
	if (!fp) {
		fprintf (stderr, "ERROR: could not open file %s\n", meta_file);
		return false;
	}
	// get header line first
	fgets (line, 128, fp);
	// loop through and get each glyph's info
	while (EOF != fscanf (
		fp, "%i %f %f %f %f %f\n",
		&ascii_code,
		&prop_xMin,
		&prop_width,
		&prop_yMin,
		&prop_height,
		&prop_y_offset
	)) {
		glyph_widths[ascii_code] = prop_width;
		glyph_y_offsets[ascii_code] = 1.0 - prop_height - prop_y_offset;
	}
	fclose (fp);
	return true;
}

bool load_font_texture (const char* file_name, GLuint* tex) {
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = NULL;
	int width_in_bytes = -1;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = -1;
	
	printf ("loading font texture from file: %s\n", file_name);
	image_data = stbi_load (file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf (stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf (
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
		);
	}
	width_in_bytes = x * 4;
	half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
	glGenTextures (1, tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, *tex);
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	return true;
}

bool create_font_shaders () {
	const char* vs_str =
	"#version 400\n"
	"in vec2 vp;"
	"in vec2 vt;"
	"out vec2 st;"
	"void main () {"
	"  st = vt;"
	"  gl_Position = vec4 (vp, 0.0, 1.0);"
	"}";
	const char* fs_str =
	"#version 400\n"
	"in vec2 st;"
	"uniform sampler2D tex;"
	"uniform vec4 text_colour;"
	"out vec4 frag_colour;"
	"void main () {"
	"  frag_colour = texture (tex, st) * text_colour;"
	"}";
	GLuint vs, fs;
	int params = -1;
	
	printf ("creating font shaders...\n");
	vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vs_str, NULL);
	glCompileShader (vs);
	glGetShaderiv (vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr,
			"ERROR: font vertex shader did not compile. check version\n");
		return false;
	}
	fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fs_str, NULL);
	glCompileShader (fs);
	glGetShaderiv (fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr,
			"ERROR: font fragment shader did not compile. check version\n");
		return false;
	}
	font_sp = glCreateProgram ();
	glAttachShader (font_sp, vs);
	glAttachShader (font_sp, fs);
	// i do this to improve support across older GL versions
	glBindAttribLocation (font_sp, 0, "vp");
	glBindAttribLocation (font_sp, 1, "vt");
	glLinkProgram (font_sp);
	glGetProgramiv (font_sp, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr,
			"ERROR: could not link font shader programme\n");
		return false;
	}
	font_sp_text_colour_loc = glGetUniformLocation (font_sp, "text_colour");
	return true;
}

bool load_font (const char* atlas_image, const char* atlas_meta) {
	if (!load_font_texture (atlas_image, &font_texture)) {
		return false;
	}
	if (!load_font_meta (atlas_meta)) {
		return false;
	}
	return true;
}

bool init_text_rendering (
	const char* font_image_file,
	const char* font_meta_data_file,
	int viewport_width,
	int viewport_height) {
	printf ("initialising text rendering...\n");
	font_viewport_width = viewport_width;
	font_viewport_height = viewport_height;
	if (!create_font_shaders ()) {
		return false;
	}
	if (!load_font (font_image_file, font_meta_data_file)) {
		return false;
	}
	return true;
}

//
// create a VBO from a string of text, using our font's glyph sizes to make a
// set of quads
void text_to_vbo (
	const char* str,
	float at_x,
	float at_y,
	float scale_px,
	GLuint* points_vbo,
	GLuint* texcoords_vbo,
	int* point_count
) {
	int len = 0;
	int i;
	float* points_tmp = NULL;
	float* texcoords_tmp = NULL;
	float line_offset = 0.0f;
	float curr_x;
	int curr_index = 0;
	
	curr_x = at_x;
	len = strlen (str);
	points_tmp = (float*)malloc (sizeof (float) * len * 12);
	texcoords_tmp = (float*)malloc (sizeof (float) * len * 12);
	for (i = 0; i < len; i++) {
		int ascii_code, atlas_col, atlas_row;
		float s, t, x_pos, y_pos;
		
		if ('\n' == str[i]) {
			line_offset += scale_px / font_viewport_height;
			curr_x = at_x;
			continue;
		}
		
		// get ascii code as integer
		ascii_code = str[i];
		
		// work out row and column in atlas
		atlas_col = (ascii_code - ' ') % ATLAS_COLS;
		atlas_row = (ascii_code - ' ') / ATLAS_COLS;
		
		// work out texture coordinates in atlas
		s = atlas_col * (1.0 / ATLAS_COLS);
		t = (atlas_row + 1) * (1.0 / ATLAS_ROWS);
		
		// work out position of glyphtriangle_width
		x_pos = curr_x;
		y_pos = at_y - scale_px / font_viewport_height *
			glyph_y_offsets[ascii_code] - line_offset;
		
		// move next glyph along to the end of this one
		if (i + 1 < len) {
			// upper-case letters move twice as far
			curr_x += glyph_widths[ascii_code] * scale_px / font_viewport_width;
		}
		// add 6 points and texture coordinates to buffers for each glyph
		points_tmp[curr_index * 12] = x_pos;
		points_tmp[curr_index * 12 + 1] = y_pos;
		points_tmp[curr_index * 12 + 2] = x_pos;
		points_tmp[curr_index * 12 + 3] = y_pos - scale_px / font_viewport_height;
		points_tmp[curr_index * 12 + 4] = x_pos + scale_px / font_viewport_width;
		points_tmp[curr_index * 12 + 5] = y_pos - scale_px / font_viewport_height;
		
		points_tmp[curr_index * 12 + 6] = x_pos + scale_px / font_viewport_width;
		points_tmp[curr_index * 12 + 7] = y_pos - scale_px / font_viewport_height;
		points_tmp[curr_index * 12 + 8] = x_pos + scale_px / font_viewport_width;
		points_tmp[curr_index * 12 + 9] = y_pos;
		points_tmp[curr_index * 12 + 10] = x_pos;
		points_tmp[curr_index * 12 + 11] = y_pos;
		
		texcoords_tmp[curr_index * 12] = s;
		texcoords_tmp[curr_index * 12 + 1] = 1.0 - t + 1.0 / ATLAS_ROWS;
		texcoords_tmp[curr_index * 12 + 2] = s;
		texcoords_tmp[curr_index * 12 + 3] = 1.0 - t;
		texcoords_tmp[curr_index * 12 + 4] = s + 1.0 / ATLAS_COLS;
		texcoords_tmp[curr_index * 12 + 5] = 1.0 - t;
		
		texcoords_tmp[curr_index * 12 + 6] = s + 1.0 / ATLAS_COLS;
		texcoords_tmp[curr_index * 12 + 7] = 1.0 - t;
		texcoords_tmp[curr_index * 12 + 8] = s + 1.0 / ATLAS_COLS;
		texcoords_tmp[curr_index * 12 + 9] = 1.0 - t + 1.0 / ATLAS_ROWS;
		texcoords_tmp[curr_index * 12 + 10] = s;
		texcoords_tmp[curr_index * 12 + 11] = 1.0 - t + 1.0 / ATLAS_ROWS;
		curr_index++;
	}
	glBindBuffer (GL_ARRAY_BUFFER, *points_vbo);
	glBufferData (
		GL_ARRAY_BUFFER,
		curr_index * 12 * sizeof (float),
		points_tmp,
		GL_DYNAMIC_DRAW
	);
	glBindBuffer (GL_ARRAY_BUFFER, *texcoords_vbo);
	glBufferData (
		GL_ARRAY_BUFFER,
		curr_index * 12 * sizeof (float),
		texcoords_tmp,
		GL_DYNAMIC_DRAW
	);
	
	free (points_tmp);
	free (texcoords_tmp);
	
	*point_count = curr_index * 6;
}

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
) {
	if (num_render_strings >= MAX_STRINGS) {
		fprintf (stderr, "ERROR: too many strings of on-screen text. max 256\n");
		return -1;
	}
	renderable_texts[num_render_strings].x = x;
	renderable_texts[num_render_strings].y = y;
	renderable_texts[num_render_strings].size_px = size_in_px;
	glGenBuffers (1, &renderable_texts[num_render_strings].points_vbo);
	glGenBuffers (1, &renderable_texts[num_render_strings].texcoords_vbo);
	text_to_vbo (str, x, y, size_in_px,
		&renderable_texts[num_render_strings].points_vbo,
		&renderable_texts[num_render_strings].texcoords_vbo,
		&renderable_texts[num_render_strings].point_count);
	// set up VAO
	glGenVertexArrays (1, &renderable_texts[num_render_strings].vao);
	glBindVertexArray (renderable_texts[num_render_strings].vao);
	glBindBuffer (GL_ARRAY_BUFFER,
		renderable_texts[num_render_strings].points_vbo);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER,
		renderable_texts[num_render_strings].texcoords_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	
	renderable_texts[num_render_strings].r = r;
	renderable_texts[num_render_strings].g = g;
	renderable_texts[num_render_strings].b = b;
	renderable_texts[num_render_strings].a = a;
	
	printf ("string %i added to render texts. point count %i\n",
		num_render_strings, renderable_texts[num_render_strings].point_count);
	
	num_render_strings++;

	return num_render_strings - 1;
}

bool update_text (int id, const char* str) {
	// just re-generate the existing VBOs and point count
	text_to_vbo (str,
		renderable_texts[id].x,
		renderable_texts[id].y,
		renderable_texts[id].size_px,
		&renderable_texts[id].points_vbo,
		&renderable_texts[id].texcoords_vbo,
		&renderable_texts[id].point_count);
	
	return true;
}

bool change_text_colour (int id, float r, float g, float b, float a) {
	renderable_texts[id].r = r;
	renderable_texts[id].g = g;
	renderable_texts[id].b = b;
	renderable_texts[id].a = a;
	return true;
}

void draw_texts () {
	int i;
	
	// always draw on-top of scene
	glDisable (GL_DEPTH_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_BLEND);
	
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, font_texture);
	glUseProgram (font_sp);
	for (i = 0; i < num_render_strings; i++) {
		glBindVertexArray (renderable_texts[i].vao);
		glUniform4f (font_sp_text_colour_loc,
			renderable_texts[i].r,
			renderable_texts[i].g,
			renderable_texts[i].b,
			renderable_texts[i].a);
		uniforms++;
		glDrawArrays (GL_TRIANGLES, 0, renderable_texts[i].point_count);
		draws++;
		verts += renderable_texts[i].point_count;
	}
	
	glEnable (GL_DEPTH_TEST);
}
