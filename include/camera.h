#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "maths_funcs.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// used for size of rear-view camera's viewport
#define CAM_TEX_WIDTH 256
#define CAM_TEX_HEIGHT 128

void init_cam ();
void recalc_perspective ();
void switch_to_rear_view ();
void switch_to_front_view ();
void move_cam (vec3 pos);

extern mat4 P;
extern mat4 V;
extern bool cam_P_dirty;
extern bool cam_V_dirty;
// texture used for rear-vision mirror. i will render a rear view to this
extern GLuint cam_mirror_tex;

#endif
