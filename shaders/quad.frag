#version 420

in vec2 st;
uniform sampler2D tex0, tex1, tex2, tex3;
out vec4 frag_colour;

void main () {
	vec4 texel0 = texture (tex0, st);
	vec4 texel1 = texture (tex1, st);
	vec4 texel2 = texture (tex2, st);
	vec4 texel3 = texture (tex3, st);
	vec4 mixa = mix (texel0, texel1, 0.5);
	vec4 mixb = mix (texel2, texel3, 0.5);
	frag_colour = mix (mixa, mixb, 0.5);
	
	// gamma correction
	frag_colour.rgb = pow (frag_colour.rgb, vec3 (1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
