#ifndef _TRAFFIC_H_
#define _TRAFFIC_H_

#include "maths_funcs.h"

struct Vehicle {
	vec3 pos;
	vec3 fwd; // forward direction
	bool in_left_lane;
	int curr_lane_marker;
};

bool init_traffic ();
bool add_truck_left_lane (int marker_num);
bool add_truck_right_lane (int marker_num);
bool update_traffic (double elapsed);
bool draw_traffic ();
bool add_left_lane_marker (vec3 pos);
bool add_right_lane_marker (vec3 pos);

#endif
