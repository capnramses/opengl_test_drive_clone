#version 410

in vec2 st;
uniform sampler2D dm;
uniform float rpm_fac;
out vec4 fc;

void main () {
	fc = texture (dm, vec2 (st.s, 1.0 - st.t));
}
