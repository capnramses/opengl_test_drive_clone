#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "maths_funcs.h"

void init_cam ();

extern mat4 P;
extern mat4 V;
extern bool cam_P_dirty;
extern bool cam_V_dirty;

#endif
