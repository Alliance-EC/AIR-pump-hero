#ifndef POWER_CONTROL_H
#define POWER_CONTROL_H
#include "main.h"
void PowerControlInit(uint16_t max_power_init, float reduction_ratio_init);
float PowerInputCalc(float motor_speed, float motor_current);
float PowerControlCalc(float power_lf, float power_lb, float power_rf, float power_rb, float motor_speed, float motor_current,int which_motor,float aiming_current);
#endif