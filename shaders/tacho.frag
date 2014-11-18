#version 420

in vec2 st;
layout (binding = 0) uniform sampler2D full_dm;
layout (binding = 1) uniform sampler2D empty_dm;
uniform float rpm_fac;
out vec4 fc;

void main () {
	if (st.s < rpm_fac) {
		fc = texture (full_dm, vec2 (st.s, 1.0 - st.t));
	} else {
		fc = texture (empty_dm, vec2 (st.s, 1.0 - st.t));
	}
}
