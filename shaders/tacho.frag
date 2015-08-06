#version 410
// you need 420 or newer to do the (binding=x) thing
// otherwise you have to do from C: glUniform1i (emtpy_dm_location, 1);

in vec2 st;
uniform sampler2D full_dm;
uniform sampler2D empty_dm;
uniform float rpm_fac;
out vec4 fc;

void main () {
	if (st.s < rpm_fac) {
		fc = texture (full_dm, vec2 (st.s, 1.0 - st.t));
	} else {
		fc = texture (empty_dm, vec2 (st.s, 1.0 - st.t));
	}
}
