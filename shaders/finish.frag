#version 420

in vec2 st;
uniform sampler2D dm;
out vec4 fc;

void main () {
	fc = texture (dm, vec2 (st.s, 1.0 - st.t));
}
