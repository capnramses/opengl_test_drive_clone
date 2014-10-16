#include "gl_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
// dimensions of the window drawing surface
int gl_width;
int gl_height;
GLFWwindow* gl_window;

//
// start opengl window
// return false on error
bool start_gl (int width, int height) {
	const GLubyte* renderer;
	const GLubyte* version;
	
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
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	gl_window = glfwCreateWindow (gl_width, gl_height, "OpenGL Test Drive Clone",
		NULL, NULL);
	if (!gl_window) {
		fprintf (stderr, "ERROR: opening OS window\n");
		return false;
	}
	glfwMakeContextCurrent (gl_window);

	glewExperimental = GL_TRUE;
	glewInit ();

	/* get version info */
	renderer = glGetString (GL_RENDERER); /* get renderer string */
	version = glGetString (GL_VERSION); /* version as a string */
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);
	
	return true;
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

	// work out size of files
	unsigned int vs_sz = get_file_size (vs_file_name);
	unsigned int fs_sz = get_file_size (fs_file_name);
	if (!vs_sz || !fs_sz) {
		return 0;
	}
	vs_str = (char*)malloc (vs_sz);
	fs_str = (char*)malloc (fs_sz);
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
	glCompileShader (fs);
	// TODO check logs
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
unsigned int get_file_size (const char* file_name) {
	FILE* fp = fopen (file_name , "r");
	if (!fp) {
		fprintf (stderr, "ERROR: opening file %s\n", file_name);
		return 0;
	}
	fseek (fp, 0, SEEK_END);
	int sz = ftell (fp);
	fclose (fp);
	return sz;
}
