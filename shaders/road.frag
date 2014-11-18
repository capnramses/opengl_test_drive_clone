#version 420

in vec2 st;
uniform sampler2D diff_map;
out vec4 frag_colour;

void main () {
	vec4 texel = texture (diff_map, st);
	frag_colour = vec4 (texel.rgb * gl_FragCoord.w * 2.5, 1.0);
	
	//
	// gamma correction
	frag_colour.rgb = pow (frag_colour.rgb, vec3 (1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
