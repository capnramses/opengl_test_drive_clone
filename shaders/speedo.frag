#version 420
// you need 420 or newer to do the (binding=x) thing
// otherwise you have to do from C: glUniform1i (emtpy_dm_location, 1);

in vec2 st;
uniform sampler2D speedo_dm;
out vec4 fc;

void main () {
	fc = texture (speedo_dm, vec2 (st.s, 1.0 - st.t));
}
