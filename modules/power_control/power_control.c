#include "power_control.h"
#include "arm_math.h"
#include "message_center.h"
#include <stdlib.h>
#include <math.h>
#include "user_lib.h"

#define MOTOR_LF 0
#define MOTOR_LB 1
#define MOTOR_RF 2
#define MOTOR_RB 3

float k1                = 1.23e-07;
float k2                = 1.453e-07;
float constant          = 4.081f;
float toque_coefficient = 1.99688994e-6f;
float reduction_ratio, total_power;
uint16_t max_power;
void PowerControlInit(uint16_t max_power_init, float reduction_ratio_init)
{
    max_power = max_power_init;
    if (reduction_ratio_init != 0) {
        reduction_ratio = reduction_ratio_init;
    } else {
        reduction_ratio = (187.0f / 3591.0f);
    }
}
float PowerInputCalc(float motor_speed, float motor_current)
{
    float power_input = motor_current * toque_coefficient * motor_speed + k2 * motor_speed * motor_speed + k1 * motor_current * motor_current + constant;
    return power_input;
}
float PowerControlCalc(float power_lf, float power_lb, float power_rf, float power_rb, float motor_speed, float motor_current, int which_motor, float aiming_current)
{
    float motor_current_output;
    float power_data[4];
    power_data[0] = power_lf;
    power_data[1] = power_lb;
    power_data[2] = power_rf;
    power_data[3] = power_rb;
    total_power   = 0;
    for (int i = 0; i < 4; i++) {
        if (power_data[i] < 0) {
            continue;
        } else {
            total_power += power_data[i];
        }
    }
    if (total_power > max_power) {
        float power_scale       = max_power / total_power;
        power_data[which_motor] = power_data[which_motor] * power_scale;
        if (power_data[which_motor] < 0) {
            return aiming_current;
        }
        float a = k1;
        float b = toque_coefficient * motor_speed;
        float c = k2 * motor_speed * motor_speed - power_data[which_motor] + constant;
        if (motor_current > 0) {
            float temp = (-b + sqrtf(b * b - 4 * a * c)) / (2 * a);
            if (temp > 15000) {
                motor_current_output = 15000;
            } else {
                motor_current_output = temp;
            }
        } else {
            float temp = (-b - sqrtf(b * b - 4 * a * c)) / (2 * a);
            if (temp < -15000) {
                motor_current_output = -15000;
            } else {
                motor_current_output = temp;
            }
        }
        return motor_current_output;
    }
    return aiming_current;
}