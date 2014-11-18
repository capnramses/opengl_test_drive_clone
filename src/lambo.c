#include <stdio.h>
#include <math.h>

//
// motor properties
float min_rpm = 2000.0f;
float max_rpm = 9000.0f;
float bottom_rpm = 2500.0f;
float top_rpm = 8000.0f;
float max_torque = 400.6f;
float min_torque = 250.0f;
float curr_motor_rpm;
float curr_motor_torque;
float throttle_fac;
bool motor_blew;

//
// transmission properties
float gear_ratios [] = {
	0.0f,
	3.5f,
	2.00f,
	1.5f,
	1.0f,
	0.75f
};
int gear_count = 6;
float diff_ratio = 3.0f;
int curr_gear;
float trans_efficiency = 0.9f;
float wheel_radius = 0.25f;
float mass = 1200.0f;

// drag etc.
float c_drag = 0.42f;
float drag_area = 0.573f;
float air_density = 1.2041; // 20 deg C 101 kPa
// 1/2pv^2CdragA

bool ch_gear_up () {
	if (curr_gear >= gear_count - 1) {
		return false;
	}
	curr_gear++;
	return true;
}

bool ch_gear_down () {
	if (curr_gear <= 0) {
		return false;
	}
	curr_gear--;
	return true;
}

//
// return torque in Nm for given motor rpm
float get_motor_torque_at (float rpm) {
	float rpm_range = max_rpm - min_rpm;
	float rpm_fac = (rpm - min_rpm) / rpm_range;
	float x = rpm_fac * M_PI;
	float t_range = max_torque - min_torque;
	return sinf (x) * t_range + min_torque;
}

//
// takes current speed
// returns wheel acceleration
float update_motor (float speed_mps) {
	//if (curr_gear == 0) {
	//	return 0.0f;
	//}

	// update motor rpm based on wheel linear speed
	// v = wr
	float w = speed_mps / wheel_radius;
	// 1 rad/s is approx 9.5493 rpms
	float wheel_rpm = 9.5493f * w;
	// note these don't use inverse of gear ratio because going backwards
	// NB do i also mult by efficiency again?
	float gear_rpm = wheel_rpm * diff_ratio;
	curr_motor_rpm = gear_rpm * gear_ratios[curr_gear];
	//printf ("rpm from speed %f\n", curr_motor_rpm);

	// make sure motor never falls below min RPM
	if (curr_motor_rpm < bottom_rpm) {
		curr_motor_rpm = bottom_rpm;
	}
	// make the motor blow up over max RPM (or change gears)
	if (curr_motor_rpm > max_rpm) {
		//printf ("BOOM! motor exploded\n");
		motor_blew = true;
		//curr_motor_rpm = top_rpm;
	}
	// set current torque as a factor of torque available at current rpm
	// throttle is the accelerator pedal between 0.0 and 1.0
	curr_motor_torque = get_motor_torque_at (curr_motor_rpm) * throttle_fac;
	// transmission
	float gear_torque = curr_motor_torque * gear_ratios[curr_gear];
	float diff_torque = gear_torque * diff_ratio;
	// Note: this is all 4 wheels combined. force at 1 powered wheel is / 2
	float wheel_torque = diff_torque * trans_efficiency;
	
	//
	// resistance forces
	// rolling (using F= Nb / r)
	float gravity = -10.0f;
	float W = fabs (mass * gravity);
	float c_rr = 0.006f; // 0.003 race tyre http://www.tribology-abc.com/abc/cof.htm
	float rr_N = (W * c_rr) / wheel_radius;
	// drag 1/2pv^2CdragA
	float drag_N = 0.5f * air_density * speed_mps * speed_mps * c_drag * drag_area;
	
	// T = r * F
	float force = wheel_torque / wheel_radius;
	force = force - rr_N - drag_N;
	// F = ma
	float accel = force / mass;
	
	return accel;
}
