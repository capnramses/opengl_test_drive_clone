#ifndef _LAMBO_H_
#define _LAMBO_H_

float get_motor_torque_at (float rpm);
float update_motor (float speed_mps);
bool ch_gear_up ();
bool ch_gear_down ();
bool set_gear (int no);

extern float curr_motor_rpm;
extern float throttle_fac;
extern int curr_gear;

#endif
