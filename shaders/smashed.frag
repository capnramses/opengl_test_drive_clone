#version 410

in vec2 st;
uniform sampler2D dm;
out vec4 fc;

void main () {
	fc = texture (dm, st);
}
