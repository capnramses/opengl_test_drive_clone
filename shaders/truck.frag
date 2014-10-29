#version 400

in vec2 st;
uniform sampler2D diff_map;
out vec4 frag_colour;

void main () {
	vec4 texel = texture (diff_map, vec2 (st.s, 1.0 - st.t));

	frag_colour = texel;
	//
	// diminished lighting
	frag_colour.rgb *= gl_FragCoord.w * 2.5;

	// gamma correction (not for texture)
	//frag_colour.rgb = pow (frag_colour.rgb, vec3 (1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
