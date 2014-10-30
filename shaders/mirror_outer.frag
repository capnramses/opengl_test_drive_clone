#version 400

out vec4 frag_colour;

void main () {
	frag_colour = vec4 (0.01, 0.01, 0.01, 1.0);
	
	//
	// gamma correction
	frag_colour.rgb = pow (frag_colour.rgb,
		vec3 (1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
