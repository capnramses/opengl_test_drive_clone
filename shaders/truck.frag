#version 400

in vec2 st;
uniform sampler2D diff_map;
out vec4 frag_colour;

void main () {
	vec4 texel = texture (diff_map, vec2 (st.s, 1.0 - st.t));

	frag_colour.rgb = texel.rgb;
	frag_colour.a = 1.0;
	//
	// diminished lighting -- modified by alpha value to allow emission maps
	float dim_fac = gl_FragCoord.w * 2.5;
	frag_colour.rgb *= mix (dim_fac, 0.8, 1.0 - texel.a);
	//frag_colour.rgb *= texel.a;
	// gamma correction (not for texture)
	//frag_colour.rgb = pow (frag_colour.rgb, vec3 (1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
