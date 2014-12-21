#include "gl_utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "camera.h"
#include "stb_image_write.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//
// dimensions of the window drawing surface
int gl_width;
int gl_height;
GLFWwindow* gl_window;
bool full_screen;

int draws;
int uniforms;
int verts;
int frames;

int myargc;
char** myargv;

unsigned char* g_video_memory_start = NULL;
unsigned char* g_video_memory_ptr = NULL;
int g_video_seconds_total = 20;
int g_video_fps = 30;

// built-in anti-aliasing to smooth jagged diagonal edges of polygons
int msaa_samples = 16;
// NOTE: if too high grainy crap appears on polygon edges

void window_resize_callback (GLFWwindow* window, int width, int height);

//
// start opengl window
// return false on error
bool start_gl (int width, int height) {
	const GLubyte* renderer;
	const GLubyte* version;
	GLFWmonitor* mon = NULL;
	
	printf ("Init OpenGL...\n");
	
	gl_width = width;
	gl_height = height;
	
	// Start OpenGL using helper libraries
	if (!glfwInit ()) {
		fprintf (stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/* change to 3.2 if on Apple OS X */
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint (GLFW_SAMPLES, msaa_samples);

	if (full_screen) {
		const GLFWvidmode* v = NULL;
		
		mon = glfwGetPrimaryMonitor ();
		v = glfwGetVideoMode (mon);
		gl_width = v->width;
		gl_height = v->height;
	}
	gl_window = glfwCreateWindow (gl_width, gl_height,
		"OpenGL Test Drive Remake", mon, NULL);

	if (!gl_window) {
		fprintf (stderr, "ERROR: opening OS window\n");
		return false;
	}
	glfwMakeContextCurrent (gl_window);
	// handle window resizing - adjust camera perspective
	glfwSetWindowSizeCallback (gl_window, window_resize_callback);
	// hide mouse
	glfwSetInputMode (gl_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glewExperimental = GL_TRUE;
	glewInit ();

	/* get version info */
	renderer = glGetString (GL_RENDERER); /* get renderer string */
	version = glGetString (GL_VERSION); /* version as a string */
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);
	
	return true;
}

void print_shader_info_log (GLuint shader_index) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog (shader_index, max_length, &actual_length, log);
	printf ("shader info log for GL index %u\n%s\n", shader_index, log);
}

//
// load a simple vertex shader + fragment shader pair from files
// and compile+link it into a new shader program
// returns handle to program
GLuint link_programme_from_files (const char* vs_file_name,
	const char* fs_file_name) {
	char* vs_str = NULL;
	char* fs_str = NULL;
	GLuint vs, fs, sp;
	long vs_sz, fs_sz;
	int params = -1;

	// work out size of files
	vs_sz = get_file_size (vs_file_name);
	fs_sz = get_file_size (fs_file_name);
	if (!vs_sz || !fs_sz) {
		return 0;
	}
	vs_str = (char*)malloc (vs_sz + 1); // oddly had to do +1 sometimes
	fs_str = (char*)malloc (fs_sz + 1); // oddly had to do +1 sometimes
	// read files intro strings
	if (!parse_file_into_str (vs_file_name, vs_str)) {
		return 0;
	}
	if (!parse_file_into_str (fs_file_name, fs_str)) {
		return 0;
	}
	// create GPU shaders
	vs = glCreateShader (GL_VERTEX_SHADER);
	fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (vs, 1, (const char**)&vs_str, NULL);
	glShaderSource (fs, 1, (const char**)&fs_str, NULL);
	free (vs_str);
	free (fs_str);
	glCompileShader (vs);
	// check for compile errors
	glGetShaderiv (vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: vertex shader %s did not compile\n", vs_file_name);
		print_shader_info_log (vs);
		return 0; // or exit or something
	}
	glCompileShader (fs);
	// check for compile errors
	glGetShaderiv (fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: frag shader %s did not compile\n", fs_file_name);
		print_shader_info_log (fs);
		return 0; // or exit or something
	}
	sp = glCreateProgram ();
	glAttachShader (sp, vs);
	glAttachShader (sp, fs);
	glLinkProgram (sp);
	
	return sp;
}

//
// copy a shader from a plain text file into a character array
// return false on error
bool parse_file_into_str (const char* file_name, char* shader_str) {
	FILE* file = fopen (file_name , "r");
	int current_len = 0;
	char line[2048];

	shader_str[0] = '\0';
	if (!file) {
		fprintf (stderr, "ERROR: opening file for reading: %s\n", file_name);
		return false;
	}
	line[0] = '\0';
	while (!feof (file)) {
		if (NULL != fgets (line, 2048, file)) {
			current_len += strlen (line); /* +1 for \n at end */
			strcat (shader_str, line);
		}
	}
	if (EOF == fclose (file)) { /* probably unnecesssary validation */
		fprintf (stderr, "ERROR: closing file from reading %s\n", file_name);
		return false;
	}
	return true;
}

//
// get a shader file's character count
// 0 on error
long get_file_size (const char* file_name) {
	FILE* fp = fopen (file_name , "r"); // could be rb for binary??
	if (!fp) {
		fprintf (stderr, "ERROR: opening file %s\n", file_name);
		return 0;
	}
	fseek (fp, 0, SEEK_END);
	long sz = ftell (fp);
	//printf ("debug: %li bytes long\n", sz);
	fclose (fp);
	return sz;
}

//
// load an image using stb_image
// create gl texture, copy it over
// return texture I.D.
GLuint create_texture_from_file (const char* file_name) {
	GLuint tex;
	int x,y,n;
	unsigned char* data;
	
	data = stbi_load (file_name, &x, &y, &n, 4);
	if (!data) {
		fprintf (stderr, "ERROR: could not load texture from %s\n", file_name);
		exit (1);
	}
	printf ("loaded image %s with [%i,%i] res and %i chans...\n",
		file_name, x, y, n);
	
	// TODO npot check and flip upside-down
	
	glGenTextures (1, &tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, tex);
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data
	);
	stbi_image_free(data);
	glGenerateMipmap (GL_TEXTURE_2D);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	float max_aniso = 0.0f;
	glGetFloatv (GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	
	return tex;
}

//
// function is automatically called when window is resized
void window_resize_callback (GLFWwindow* window, int width, int height) {
	printf ("window addr.%p resized to %ix%i\n", (void*)window, width, height);
	gl_width = width;
	gl_height = height;
	// recalculate projection matrix here
	recalc_perspective ();
}

void reserve_video_memory () {
	// 480 MB at 800x800 resolution 230.4 MB at 640x480 resolution
	g_video_memory_ptr = (unsigned char*)malloc (
		gl_width * gl_height * 3 * g_video_fps * g_video_seconds_total
	);
	g_video_memory_start = g_video_memory_ptr;
}

void grab_video_frame () {
	// copy frame-buffer into 24-bit rgbrgb...rgb image
	glReadPixels (
		0, 0, gl_width, gl_height, GL_RGB, GL_UNSIGNED_BYTE, g_video_memory_ptr
	);
	// move video pointer along to the next frame's worth of bytes
	g_video_memory_ptr += gl_width * gl_height * 3;
}

bool dump_video_frame () {
	static long int frame_number = 0;
	printf ("writing video frame %li\r", frame_number);
	// write into a file
	char name[1024];
	sprintf (name, "video_frame_%03ld.png", frame_number);
	
	unsigned char* last_row = g_video_memory_ptr +
		(gl_width * 3 * (gl_height - 1));
	if (!stbi_write_png (name, gl_width, gl_height, 3, last_row, -3 * gl_width)) {
		fprintf (stderr, "ERROR: could not write video file %s\n", name);
		return false;
	}

	frame_number++;
	return true;
}

bool dump_video_frames () {
	// reset iterating pointer first
	g_video_memory_ptr = g_video_memory_start;
	printf ("\n");
	for (int i = 0; i < g_video_seconds_total * g_video_fps; i++) {
		if (!dump_video_frame ()) {
			return false;
		}
		g_video_memory_ptr += gl_width * gl_height * 3;
	}
	printf ("\n");
	free (g_video_memory_start);
	printf ("VIDEO IMAGES DUMPED\n");
	return true;
}

//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
int M_CheckParm (const char *check) {
	int i;
	for (i = 1; i < myargc; i++) {
		if (!strcasecmp(check, myargv[i])) {
			return i;
		}
	}
	return 0;
}

//
// check a framebuffer was created properly
bool verify_bound_framebuffer () {
	GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != status) {
		fprintf (stderr, "ERROR: incomplete framebuffer\n");
		if (GL_FRAMEBUFFER_UNDEFINED == status) {
			fprintf (stderr, "GL_FRAMEBUFFER_UNDEFINED\n");
		} else if (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT == status) {
			fprintf (stderr, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
		} else if (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT == status) {
			fprintf (stderr, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
		} else if (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER == status) {
			fprintf (stderr, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");
		} else if (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER== status) {
			fprintf (stderr, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n");
		} else if (GL_FRAMEBUFFER_UNSUPPORTED == status) {
			fprintf (stderr, "GL_FRAMEBUFFER_UNSUPPORTED\n");
		} else if (GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE == status) {
			fprintf (stderr, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n");
		} else if (GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS == status) {
			fprintf (stderr, "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n");
		} else {
			fprintf (stderr, "unspecified error - this shouldn't happen.\n");
		}
		return false;
	}
	printf ("framebuffer is complete\n");
	return true;
}

//
// using Sean Barrett's stb_image_write.h to quickly convert to PNG
bool screenshot () {
	unsigned char* buffer = (unsigned char*)malloc (gl_width * gl_height * 3);
	glReadPixels (
		0, 0, gl_width, gl_height, GL_RGB, GL_UNSIGNED_BYTE, buffer
	);
	if (!buffer) {
		fprintf (stderr, "ERROR: could not allocate memory for screen capture\n");
		return false;
	}
	char name[1024];
	long int t = time (NULL);
	sprintf (name, "screenshot_%ld.png", t);
	unsigned char* last_row = buffer + (gl_width * 3 * (gl_height - 1));
	if (!stbi_write_png (
		name, gl_width, gl_height, 3, last_row, -3 * gl_width
	)) {
		fprintf (stderr, "ERROR: could not write screenshot file %s\n", name);
		return false;
	}
	free (buffer);
	return true;
}

