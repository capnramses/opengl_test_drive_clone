#include "camera.h"
#include "gl_utils.h"
#include <assert.h>

mat4 P;
mat4 V;
// texture that secondary framebuffer will render to
GLuint cam_mirror_tex;
// secondary framebuffer
GLuint cam_mirror_fb;

bool cam_P_dirty = false;
bool cam_V_dirty = false;
bool is_forward_cam = true;

vec3 cam_pos = vec3 (0.0f, 1.0f, 0.0f);
float fovy = 67.5f;
float near = 0.1f;
float far = 200.0f;

vec3 targ, targ_inv;

void init_cam () {
	vec3 up;
	
	targ = cam_pos + vec3 (0.0f, 0.0f, -1.0f);
	up = normalise (vec3 (0.0f, 1.0f, -0.1f));
	V = look_at (cam_pos, targ, up);
	cam_V_dirty = true;
	
	recalc_perspective ();
	
	//
	// set up secondary render-to-texture framebuffer for the rear-vision mirror
	// view
	glGenFramebuffers (1, &cam_mirror_fb);
	glBindFramebuffer (GL_FRAMEBUFFER, cam_mirror_fb);
	// create depth texture (enabled depth-sorting in a secondary framebuffer)
	GLuint depth_tex;
	glGenTextures (1, &depth_tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, depth_tex);
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		CAM_TEX_WIDTH,
		CAM_TEX_HEIGHT,
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
	glGenTextures (1, &cam_mirror_tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, cam_mirror_tex);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// make the texture the same size as the viewport
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_SRGB_ALPHA,
		CAM_TEX_WIDTH,
		CAM_TEX_HEIGHT,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		NULL
	);
	// attach texture to fb
	glFramebufferTexture2D (
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cam_mirror_tex, 0);
	// redirect fragment shader output 0 used to the texture that we just bound
	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers (1, draw_bufs);
	
	// check for completeness
	assert (verify_bound_framebuffer ());

	// bind the default fb back
	glBindFramebuffer (GL_FRAMEBUFFER, 0);

	targ = cam_pos + vec3 (0.0f, 0.0f, -1.0);
}

void recalc_perspective () {
	float aspect = 1.0f;
	
	aspect = (float)gl_width / (float)gl_height;
	P = perspective (fovy, aspect, near, far);
	cam_P_dirty = true;
}

void set_fovy (float _fovy) {
	float aspect = 1.0f;
	
	fovy = _fovy;
	aspect = (float)gl_width / (float)gl_height;
	P = perspective (fovy, aspect, near, far);
	cam_P_dirty = true;
}

void switch_to_rear_view () {
	float aspect = 1.0f;
	is_forward_cam = false;
	
	move_cam (cam_pos);
	
	aspect = 2.5f / 1.0f;
	P = perspective (35.0f, aspect, near, far);
	cam_P_dirty = true;
	glBindFramebuffer (GL_FRAMEBUFFER, cam_mirror_fb);
}

void switch_to_front_view () {
	is_forward_cam = true;
	move_cam (cam_pos);
	recalc_perspective ();
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

void move_cam (vec3 pos) {
	if (is_forward_cam) {
		V = look_at (pos, targ, vec3 (0.0f, 1.0f, 0.0f));
	} else {
		V = look_at (pos + vec3 (0.0f, 0.25f, 0.0f), targ_inv, vec3 (0.0f, 1.0f, 0.0f));
	}
	cam_pos = pos;
	cam_V_dirty = true;
}

void set_heading (float deg) {
	vec4 t = vec4 (0.0f, 0.0f, -1.0f, 1.0f);
	targ = vec3 (rotate_y_deg (identity_mat4 (), deg) * t) + cam_pos;
	t = vec4 (0.0f, 0.0f, 1.0f, 1.0f);
	targ_inv = vec3 (rotate_y_deg (identity_mat4 (), deg) * t) + cam_pos;
}
