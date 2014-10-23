#include "camera.h"
#include "gl_utils.h"

mat4 P;
mat4 V;

bool cam_P_dirty = false;
bool cam_V_dirty = false;

vec3 cam_pos = vec3 (0.0f, 1.0f, 0.0f);
float fovy = 67.5f;
float near = 0.1f;
float far = 200.0f;

void init_cam () {
	vec3 targ_pos, up;
	
	targ_pos = cam_pos + vec3 (0.0f, 0.0f, -10.0f);
	up = normalise (vec3 (0.0f, 1.0f, -0.1f));
	V = look_at (cam_pos, targ_pos, up);
	cam_V_dirty = true;
	
	recalc_perspective ();
}

void recalc_perspective () {
	float aspect = 1.0f;
	
	aspect = (float)gl_width / (float)gl_height;
	P = perspective (fovy, aspect, near, far);
	cam_P_dirty = true;
}
