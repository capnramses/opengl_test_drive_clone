#version 420

in vec2 st;
uniform sampler2D mirror_map;
out vec4 frag_colour;

void main () {
	frag_colour = texture (mirror_map, vec2 (1.0 - st.s, st.t));
	
	// darken mirror
	frag_colour.rgb *= 0.8;
	
	//
	// gamma correction
	frag_colour.rgb = pow (frag_colour.rgb,
		vec3 (1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
