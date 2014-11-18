#ifndef _DASH_H_
#define _DASH_H_

#include "maths_funcs.h"

bool init_dash ();
void draw_dash ();
void move_dash (vec3 p);
void set_steering (float deg);
void set_rpm_fac (float fac);

extern bool draw_smashed;

#endif
