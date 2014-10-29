#ifndef _TRAFFIC_H_
#define _TRAFFIC_H_

#include "maths_funcs.h"

bool init_traffic ();
bool add_truck (vec3 start_pos);
bool update_traffic (double elapsed);
bool draw_traffic ();

#endif
