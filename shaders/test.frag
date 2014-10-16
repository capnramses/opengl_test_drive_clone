#version 400

in vec2 st;
uniform sampler2D diff_map;
out vec4 frag_colour;

void main () {
	vec4 texel = texture (diff_map, st);
	frag_colour = vec4 (texel.rgb, 1.0);
}
