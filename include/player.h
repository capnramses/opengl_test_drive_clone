#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "maths_funcs.h"

void update_player (double elapsed);
bool finished_level ();
vec3 get_player_pos ();

extern bool det_joystick;

#endif
