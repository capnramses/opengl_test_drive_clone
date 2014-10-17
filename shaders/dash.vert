#version 400

layout (location=0) in vec3 vp;
layout (location=1) in vec2 vt;
layout (location=2) in vec3 vn;
uniform mat4 P, V;
out vec2 st;
out vec3 n_eye;

void main () {
	gl_Position = P * V * vec4 (vp.x, vp.y + 1.0, vp.z - 1.2, 1.0);
	st = vt;
	n_eye = (V * vec4 (vn, 0.0)).xyz;
}
