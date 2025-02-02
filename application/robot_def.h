/**
 * @file robot_def.h
 * @author NeoZeng neozng1@hnu.edu.cn
 * @author Even
 * @version 0.1
 * @date 2022-12-02
 *
 * @copyright Copyright (c) HNU YueLu EC 2022 all rights reserved
 *
 */
#pragma once // 可以用#pragma once代替#ifndef ROBOT_DEF_H(header guard)
#ifndef ROBOT_DEF_H
#define ROBOT_DEF_H

#include "ins_task.h"
#include "master_process.h"
#include "stdint.h"
/* 开发板类型定义,烧录时注意不要弄错对应功能;修改定义后需要重新编译,只能存在一个定义! */
// #define ONE_BOARD // 单板控制整车ONE_BOARD
#define CHASSIS_BOARD //底盘板
//  #define GIMBAL_BOARD    // 云台板
#define VISION_USE_VCP // 使用虚拟串口发送视觉数据
// #define VISION_USE_UART // 使用串口发送视觉数据
// 
/* 机器人重要参数定义,注意根据不同机器人进行修改,浮点数需要以.0或f结尾,无符号以u结尾 */
// 云台参数
#define YAW_CHASSIS_ALIGN_ECD     1296  // 云台和底盘对齐指向相同方向时的电机编码器值,若对云台有机械改动需要修改
#define YAW_ECD_GREATER_THAN_4096 0     // ALIGN_ECD值是否大于4096,是为1,否为0;用于计算云台偏转角度
#define PITCH_HORIZON_ECD         7979  // 云台处于水平位置时编码器值,若对云台有机械改动需要修改
#define PITCH_MAX_ANGLE           0.71  // 云台竖直方向最大角度 (注意反馈如果是陀螺仪，则填写陀螺仪的角度)
#define PITCH_MIN_ANGLE           -0.63 // 云台竖直方向最小角度 (注意反馈如果是陀螺仪，则填写陀螺仪的角度)
#define PITCH_MAX_ANGLE_motor           4588  // 云台竖直方向最大角度 (注意反馈如果是陀螺仪，则填写陀螺仪的角度)
#define PITCH_MIN_ANGLE_motor           6251 // 云台竖直方向最小角度 (注意反馈如果是陀螺仪，则填写陀螺仪的角度)
// 将编码器转换成角度值

#define YAW_ALIGN_ANGLE     (YAW_CHASSIS_ALIGN_ECD * ECD_ANGLE_COEF_DJI) // 对齐时的角度,0-360
#define PTICH_HORIZON_ANGLE (PITCH_HORIZON_ECD * ECD_ANGLE_COEF_DJI)     // pitch水平时电机的角度,0-360
// 发射参数
#define ONE_BULLET_DELTA_ANGLE 70    // 发射一发弹丸拨盘转动的距离,由机械设计图纸给出
#define REDUCTION_RATIO_LOADER 49.0f // 拨盘电机的减速比,英雄需要修改为3508的19.0f
#define NUM_PER_CIRCLE         10    // 拨盘一圈的装载量
// 机器人底盘修改的参数,单位为mm(毫米)
#define WHEEL_BASE             630    // 纵向轴距(前进后退方向)
#define TRACK_WIDTH            462    // 横向轮距(左右平移方向)
#define CENTER_GIMBAL_OFFSET_X 0      // 云台旋转中心距底盘几何中心的距离,前后方向,云台位于正中心时默认设为0
#define CENTER_GIMBAL_OFFSET_Y 0      // 云台旋转中心距底盘几何中心的距离,左右方向,云台位于正中心时默认设为0
#define RADIUS_WHEEL           76.25f // 轮子半径
#define REDUCTION_RATIO_WHEEL  19.2f  // 电机减速比,因为编码器量测的是转子的速度而不是输出轴的速度故需进行转换

// 陀螺仪校准数据，开启陀螺仪校准后可从INS中获取
#define BMI088_PRE_CALI_GYRO_X_OFFSET -0.000824903022
#define BMI088_PRE_CALI_GYRO_Y_OFFSET 0.000809144345
#define BMI088_PRE_CALI_GYRO_Z_OFFSET 3.52914867e-05
// 陀螺仪默认环境温度
#define BMI088_AMBIENT_TEMPERATURE 25.0f

#define GYRO2GIMBAL_DIR_YAW        1 // 陀螺仪数据相较于云台的yaw的方向,1为相同,-1为相反
#define GYRO2GIMBAL_DIR_PITCH      1 // 陀螺仪数据相较于云台的pitch的方向,1为相同,-1为相反
#define GYRO2GIMBAL_DIR_ROLL       1 // 陀螺仪数据相较于云台的roll的方向,1为相同,-1为相反
// 超级电容宏定义
#define SUPERCAP_VOLTAGE_ENOUGH             (1)
#define SUPERCAP_VOLTAGE_ENOUGH_NOT         (0)

#define SUPERCAP_RELAY_FLAG_FROM_USER_OPEN  (1)
#define SUPERCAP_RELAY_FLAG_FROM_USER_CLOSE (0)

#define SUPERCAP_RELAY_FLAG_FROM_CAN_OPEN   (1)
#define SUPERCAP_RELAY_FLAG_FROM_CAN_CLOSE  (0)

#define SUPERCAP_OPEN_FLAG_FROM_REAL_OPEN   (0)
#define SUPERCAP_OPEN_FLAG_FROM_REAL_CLOSE  (1)

#define SUPER_VOLT_MIN                      (12.0f)
// 其他参数(尽量所有参数集中到此文件)
#define BUZZER_SILENCE 0 // 蜂鸣器静音,1为静音,0为正常

// 检查是否出现主控板定义冲突,只允许一个开发板定义存在,否则编译会自动报错
#if (defined(ONE_BOARD) && defined(CHASSIS_BOARD)) || \
    (defined(ONE_BOARD) && defined(GIMBAL_BOARD)) ||  \
    (defined(CHASSIS_BOARD) && defined(GIMBAL_BOARD))
#error Conflict board definition! You can only define one board type.
#endif

#pragma pack(1) // 压缩结构体,取消字节对齐,下面的数据都可能被传输
/* -------------------------基本控制模式和数据类型定义-------------------------*/
/**
 * @brief 这些枚举类型和结构体会作为CMD控制数据和各应用的反馈数据的一部分
 *
 */
// 机器人状态
typedef enum {
    ROBOT_STOP = 0,
    ROBOT_READY,
} Robot_Status_e;

// 应用状态
typedef enum {
    APP_OFFLINE = 0,
    APP_ONLINE,
    APP_ERROR,
} App_Status_e;

// 底盘模式设置
/**
 * @brief 后续考虑修改为云台跟随底盘,而不是让底盘去追云台,云台的惯量比底盘小.
 *
 */
typedef enum {
    CHASSIS_ZERO_FORCE = 0,    // 电流零输入
    CHASSIS_ROTATE,            // 小陀螺模式
    CHASSIS_NO_FOLLOW,         // 不跟随，允许全向平移
    CHASSIS_FOLLOW_GIMBAL_YAW, // 跟随模式，底盘叠加角度环控制
} chassis_mode_e;

// 云台模式设置
typedef enum {
    GIMBAL_ZERO_FORCE = 0, // 电流零输入
    GIMBAL_FREE_MODE,      // 云台自由运动模式,即与底盘分离(底盘此时应为NO_FOLLOW)反馈值为电机total_angle;似乎可以改为全部用IMU数据?
    GIMBAL_GYRO_MODE,      // 云台陀螺仪反馈模式,反馈值为陀螺仪pitch,total_yaw_angle,底盘可以为小陀螺和跟随模式
} gimbal_mode_e;

// 发射模式设置
typedef enum {
    SHOOT_OFF = 0,
    SHOOT_ON,
} shoot_mode_e;
typedef enum {
    FRICTION_OFF = 0, // 摩擦轮关闭
    FRICTION_ON,      // 摩擦轮开启
} friction_mode_e;

typedef enum {
    LID_OPEN = 0, // 弹舱盖打开
    LID_CLOSE,    // 弹舱盖关闭
} lid_mode_e;

typedef enum {
    SIGHT_OFF = 0,
    SIGHT_ON, // 瞄准镜打开
} sight_mode_e;

typedef enum {
    Follow_shoot = 0,
    snipe,
    sentry,
} Servo_Motor_mode_e; // 图传舵机状态
typedef enum {
    VISION_OFF,
    VISION_ON,
} Vision_Shoot_mode_e;
typedef struct
{ // 功率控制
    float chassis_power_mx;
} Chassis_Power_Data_s;

/* ----------------CMD应用发布的控制数据,应当由gimbal/chassis/shoot订阅---------------- */
/**
 * @brief 对于双板情况,遥控器和pc在云台,裁判系统在底盘
 *
 */
// cmd发布的底盘控制数据,由chassis订阅
typedef struct
{
    float vx;           // 前进方向速度
    float vy;           // 横移方向速度
    float wz;           // 旋转速度
    float offset_angle; // 底盘和归中位置的夹角
    chassis_mode_e chassis_mode;
    int chassis_speed_buff;
    // UI部分
    //  ...

} Chassis_Ctrl_Cmd_s;

// cmd发布的云台控制数据,由gimbal订阅
typedef struct
{ // 云台角度控制
    float yaw;
    float pitch;
    gimbal_mode_e gimbal_mode;
    Servo_Motor_mode_e image_mode;
    sight_mode_e sight_mode;
    Vision_Shoot_mode_e vision_mode;
} Gimbal_Ctrl_Cmd_s;
typedef enum { // 排气阀开关
    AIR_PUMP_OFF = 0,
    AIR_PUMP_ON,
} Air_Pump_mode;

typedef enum { // 发射步骤
    LOAD = 0,
    PUSH,
    FIRE,
} Shoot_Step;
typedef enum { // 拨弹盘状态
    LOAD_STOP,
    LOAD_ON,
} Loader_mode_e;
typedef enum { // 光电门状态
    BULLET_READY = 0,
    BULLET_NO_READY,
} photogate_state_e;
typedef struct
{ // 云台角度控制
    Shoot_Step now_step;
    int8_t START_flag;
} One_shoot_control;
typedef enum
{ // 云台角度控制
    POWER_ON,
    POWER_OFF,
} Shoot_Power;
typedef enum
{ 
    IMU_CONTROL,
    ANGLE_CONTROL,
} Gimbal_controll_mode;
// cmd发布的发射控制数据,由shoot订阅
typedef struct
{
    Shoot_Power Shoot_power;
    shoot_mode_e shoot_mode;
    Loader_mode_e load_mode;
    Air_Pump_mode air_pump_mode;

} Shoot_Ctrl_Cmd_s;

/* ----------------gimbal/shoot/chassis发布的反馈数据----------------*/
/**
 * @brief 由cmd订阅,其他应用也可以根据需要获取.
 *
 */

typedef struct
{
#if defined(CHASSIS_BOARD) || defined(GIMBAL_BOARD) // 非单板的时候底盘还将imu数据回传(若有必要)
    // attitude_t chassis_imu_data;
#endif
    // 后续增加底盘的真实速度
    // float real_vx;
    // float real_vy;
    // float real_wz;
    Gimbal_Ctrl_Cmd_s gimbal_cmd_upload;
    Shoot_Ctrl_Cmd_s shoot_cmd_upload;
    uint8_t rest_heat; // 剩余枪口热量
    // Bullet_Speed_e bullet_speed; // 弹速限制
    Enemy_Color_e enemy_color; // 0 for blue, 1 for red
    // float pre_power,real_power;
    // int16_t current1,speed_aps,current2,current3,current4;
} Chassis_Upload_Data_s;

typedef struct
{
    float Pitch_data;
    float yaw_motor_single_round_angle;
} Gimbal_Upload_Data_s;

typedef struct
{
    Shoot_Step bullet_ready;
    // code to go here
    // ...
} Shoot_Upload_Data_s;

typedef struct
{
    Gimbal_Upload_Data_s Gimbal_data;
    Shoot_Upload_Data_s Shoot_data;
} Upboard_data;

typedef struct
{
    uint8_t tag; /* 数据标签:0x91 */
    uint8_t id;  /* 模块ID */
    uint8_t rev[2];
    float prs;     /* 气压 */
    uint32_t ts;   /* 时间戳 */
    float acc[3];  /* 加速度 */
    float gyr[3];  /* 角速度 */
    float mag[3];  /* 地磁 */
    float eul[3];  /* 欧拉角: Roll,Pitch,Yaw */
    float quat[4]; /* 四元数 */
} IMU_data_outside;

#pragma pack() // 开启字节对齐,结束前面的#pragma pack(1)

#endif // !ROBOT_DEF_H