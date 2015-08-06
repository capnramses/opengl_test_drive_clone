#version 410

out vec4 frag_colour;

void main () {
	vec3 colour = vec3 (0.0, 1.0, 1.0);
	
	frag_colour = vec4 (colour , 1.0);
	
	//
	// gamma correction
	frag_colour.rgb = pow (frag_colour.rgb, vec3 (
		1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
