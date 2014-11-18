#version 420

in vec2 st;
in vec3 n_eye;
uniform float h, w;
out vec4 frag_colour;

void main () {
	// retrieve normals
	vec3 n = normalize (n_eye);

	vec3 colour = vec3 (0.0, 1.0, 1.0);
	
	//
	// colour affected by face direction (crappy pseudo-lighting)
	float factor = dot (n, vec3 (0.0, 0.0, 1.0)) * 0.8;
	colour.rgb *= (factor + 0.2);
	
	//
	// do that nasty purple hatching effect from the CGA palette
	int grain_size = 4;
	int grain_div = 2;
	if (w > 800 || h > 800) {
		grain_size = 8;
		grain_div = 4;
	}
	int x_ss = int (gl_FragCoord.x);
	int y_ss = int (gl_FragCoord.y);
	if (x_ss % grain_size < grain_div) {
		if (y_ss % grain_size >= grain_div) {
			colour = vec3 (0.0, 0.2, 0.2);
		}
	}
	if (x_ss % grain_size >= grain_div) {
		if (y_ss % grain_size < grain_div) {
			colour = vec3 (0.0, 0.2, 0.2);
		}
	}
	frag_colour = vec4 (colour , 1.0);
	
	//
	// gamma correction
	frag_colour.rgb = pow (frag_colour.rgb, vec3 (1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
