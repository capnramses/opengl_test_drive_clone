#version 400

in vec2 vp;
out vec2 st;

void main () {
	gl_Position = vec4 (vp, 0.0, 1.0);
	st = vec2 (vp.xy * 0.5 + 0.5);
}
