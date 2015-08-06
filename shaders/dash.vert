#version 410

layout (location=0) in vec3 vp;
layout (location=1) in vec2 vt;
layout (location=2) in vec3 vn;
uniform mat4 P, V, M;
out vec2 st;
out vec3 n_eye;

void main () {
	gl_Position = P * V * M * vec4 (vp, 1.0);
	st = vt;
	n_eye = (V * M * vec4 (vn, 0.0)).xyz;
}
