#version 420

in vec2 st;
uniform float h, w;
out vec4 frag_colour;

void main () {
	vec3 colour = vec3 (1.0, 0.0, 1.0);
	
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
			colour = vec3 (0.0, 0.0, 0.0);
		}
	}
	if (x_ss % grain_size >= grain_div) {
		if (y_ss % grain_size < grain_div) {
			colour = vec3 (0.0, 0.0, 0.0);
		}
	}
	
	
	frag_colour = vec4 (colour * gl_FragCoord.w * 2.5, 1.0);
	
	//
	// gamma correction
	frag_colour.rgb = pow (frag_colour.rgb, vec3 (1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
