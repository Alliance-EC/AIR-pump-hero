/**
 * @file chassis.c
 * @author NeoZeng neozng1@hnu.edu.cn
 * @brief 底盘应用,负责接收robot_cmd的控制命令并根据命令进行运动学解算,得到输出
 *        注意底盘采取右手系,对于平面视图,底盘纵向运动的正前方为x正方向;横向运动的右侧为y正方向
 *
 * @version 0.1
 * @date 2022-12-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "chassis.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "super_cap.h"
#include "message_center.h"
#include "referee_init.h"
#include "UI.h"
#include "general_def.h"
#include "bsp_dwt.h"
#include "referee_UI.h"
#include "arm_math.h"
#include "power_calc.h"
/* 根据robot_def.h中的macro自动计算的参数 */
#define HALF_WHEEL_BASE  (WHEEL_BASE / 2.0f)     // 半轴距
#define HALF_TRACK_WIDTH (TRACK_WIDTH / 2.0f)    // 半轮距
#define PERIMETER_WHEEL  (RADIUS_WHEEL * 2 * PI) // 轮子周长

/* 底盘应用包含的模块和信息存储,底盘是单例模式,因此不需要为底盘建立单独的结构体 */
#ifdef CHASSIS_BOARD // 如果是底盘板,使用板载IMU获取底盘转动角速度
#include "can_comm.h"
#include "ins_task.h"
CANCommInstance *chassis_can_comm; // 双板通信CAN comm
static INS_Instance *Chassis_IMU_data;
#endif                                      // CHASSIS_BOARD
static Publisher_t *chassis_pub;            // 用于发布底盘的数据
static Subscriber_t *chassis_sub;           // 用于订阅底盘的控制命令                                        // !ONE_BOARD
static Chassis_Ctrl_Cmd_s chassis_cmd_recv; // 底盘接收到的控制命令
// static Chassis_Upload_Data_s chassis_feedback_data; // 底盘回传的反馈数据
static Publisher_t *referee_pub;
static Publisher_t *Cap_pub;
referee_info_t *referee_data; // 用于获取裁判系统的数据

SuperCapInstance *cap;                                       // 超级电容
DJIMotorInstance *motor_lf, *motor_rf, *motor_lb, *motor_rb; // left right forward back
// static Chassis_Power_Data_s chassis_power_data;

/* 用于自旋变速策略的时间变量 */
// static float t;

/* 私有函数计算的中介变量,设为静态避免参数传递的开销 */
static float chassis_vx, chassis_vy;     // 将云台系的速度投影到底盘
static float vt_lf, vt_rf, vt_lb, vt_rb; // 底盘速度解算后的临时输出,待进行限幅
static Chassis_Upload_Data_s chassis_feedback_data;
extern uint8_t Super_flag;
void ChassisInit()
{
    // 修改减速比以及最大功率
    // 四个轮子的参数一样,改tx_id和反转标志位即可
    Motor_Init_Config_s chassis_motor_config = {
        .can_init_config.can_handle   = &hcan1,
        .controller_param_init_config = {
            .speed_PID = {
                .Kp            = 0.5, // 4.5
                .Ki            = 0,   // 0
                .Kd            = 0,   // 0
                .IntegralLimit = 3000,
                .Improve       = PID_Trapezoid_Intergral | PID_Integral_Limit | PID_Derivative_On_Measurement,
                .MaxOut        = 15000,
                0},
            .current_PID = {
                .Kp            = 0.5, // 0.4
                .Ki            = 0,   // 0
                .Kd            = 0,
                .IntegralLimit = 3000,
                .Improve       = PID_Trapezoid_Intergral | PID_Integral_Limit | PID_Derivative_On_Measurement,
                .MaxOut        = 15000,
            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED, .speed_feedback_source = MOTOR_FEED, .outer_loop_type = SPEED_LOOP, .close_loop_type = SPEED_LOOP,
            //.feedback_reverse_flag    = FEEDBACK_DIRECTION_REVERSE,
        },
        .motor_type = M3508,
    };
    //  @todo: 当前还没有设置电机的正反转,仍然需要手动添加reference的正负号,需要电机module的支持,待修改.
    chassis_motor_config.can_init_config.tx_id                             = 1;
    chassis_motor_config.controller_setting_init_config.motor_reverse_flag = MOTOR_DIRECTION_REVERSE;
    motor_lf                                                               = DJIMotorInit(&chassis_motor_config);

    chassis_motor_config.can_init_config.tx_id                             = 2;
    chassis_motor_config.controller_setting_init_config.motor_reverse_flag = MOTOR_DIRECTION_REVERSE;
    motor_rf                                                               = DJIMotorInit(&chassis_motor_config);

    chassis_motor_config.can_init_config.tx_id                             = 3;
    chassis_motor_config.controller_setting_init_config.motor_reverse_flag = MOTOR_DIRECTION_REVERSE;
    motor_rb                                                               = DJIMotorInit(&chassis_motor_config);

    chassis_motor_config.can_init_config.tx_id                             = 4;
    chassis_motor_config.controller_setting_init_config.motor_reverse_flag = MOTOR_DIRECTION_REVERSE;
    motor_lb                                                               = DJIMotorInit(&chassis_motor_config);

    referee_data = RefereeHardwareInit(&huart6); // 裁判系统初始化,会同时初始化UI

    SuperCap_Init_Config_s cap_conf = {
        .can_config = {
            .can_handle = &hcan1,
            .tx_id      = 0X427, // 超级电容默认接收id
            .rx_id      = 0X300  // 超级电容默认发送id,注意tx和rx在其他人看来是反的
        }};
    cap = SuperCapInit(&cap_conf); // 超级电容初始化

    // 发布订阅初始化,如果为双板,则需要can comm来传递消息
#ifdef CHASSIS_BOARD
    //  Chassis_IMU_data = INS_Init(BMI088Register(&config)); // 底盘IMU初始化

    CANComm_Init_Config_s comm_conf = {
        .can_config = {
            .can_handle = &hcan2,
            .tx_id      = 0x311,
            .rx_id      = 0x312,
        },
        .recv_data_len = sizeof(Chassis_Ctrl_Cmd_s),
        .send_data_len = sizeof(Chassis_Upload_Data_s),
    };
    chassis_can_comm = CANCommInit(&comm_conf); // can comm初始化
#endif                                          // CHASSIS_BOARD
    chassis_sub = SubRegister("chassis_cmd", sizeof(Chassis_Ctrl_Cmd_s));
    chassis_pub = PubRegister("chassis_feed", sizeof(Chassis_Upload_Data_s));
    referee_pub = PubRegister("referee_data", sizeof(referee_info_t));
    Cap_pub     = PubRegister("cap_data", sizeof(SuperCapInstance));
    PubPushMessage(referee_pub, (void *)referee_data);
}

#define LF_CENTER ((HALF_TRACK_WIDTH + CENTER_GIMBAL_OFFSET_X + HALF_WHEEL_BASE - CENTER_GIMBAL_OFFSET_Y) * DEGREE_2_RAD)
#define RF_CENTER ((HALF_TRACK_WIDTH - CENTER_GIMBAL_OFFSET_X + HALF_WHEEL_BASE - CENTER_GIMBAL_OFFSET_Y) * DEGREE_2_RAD)
#define LB_CENTER ((HALF_TRACK_WIDTH + CENTER_GIMBAL_OFFSET_X + HALF_WHEEL_BASE + CENTER_GIMBAL_OFFSET_Y) * DEGREE_2_RAD)
#define RB_CENTER ((HALF_TRACK_WIDTH - CENTER_GIMBAL_OFFSET_X + HALF_WHEEL_BASE + CENTER_GIMBAL_OFFSET_Y) * DEGREE_2_RAD)
/**
 * @brief 计算每个轮毂电机的输出,正运动学解算
 *        用宏进行预替换减小开销,运动解算具体过程参考教程
 */
static void MecanumCalculate()
{
    vt_lf = -chassis_vx - chassis_vy - chassis_cmd_recv.wz * LF_CENTER;
    vt_rf = -chassis_vx + chassis_vy - chassis_cmd_recv.wz * RF_CENTER;
    vt_lb = chassis_vx - chassis_vy - chassis_cmd_recv.wz * LB_CENTER;
    vt_rb = chassis_vx + chassis_vy - chassis_cmd_recv.wz * RB_CENTER;
}

/** 吗
 * @brief 根据裁判系统和电容剩余容量对输出进行限制并设置电机参考值
 *
 */
float lf_limit, rf_limit, lb_limit, rb_limit;
static float Power_Max = 60.0f;
static void LimitChassisOutput()
{
    // 功率限制待添加
    // referee_data->PowerHeatData.chassis_power;
    // referee_data->PowerHeatData.chassis_power_buffer;
    // power->physical_quantity.max_power = referee_data->GameRobotState.chassis_power_limit;

    // lf_limit = PowerControlCalc(power, motor_lf->measure.real_current, motor_lf->measure.speed_aps);
    // rf_limit = PowerControlCalc(power, motor_rf->measure.real_current, motor_rf->measure.speed_aps);
    // lb_limit = PowerControlCalc(power, motor_lb->measure.real_current, motor_lb->measure.speed_aps);
    // rb_limit = PowerControlCalc(power, motor_rb->measure.real_current, motor_rb->measure.speed_aps);

    // if (abs(vt_lf) > abs(lf_limit)) {
    //     vt_lf = lf_limit;
    // }
    // if (abs(vt_rf) > abs(rf_limit)) {
    //     vt_rf = rf_limit;
    // }
    // if (abs(vt_lb) > abs(lb_limit)) {
    //     vt_lb = lb_limit;
    // }
    // if (abs(vt_rb) > abs(rb_limit)) {
    //     vt_rb = rb_limit;
    // }新版功率控制待添加

    // 完成功率限制后进行电机参考输入设定
    DJIMotorSetRef(motor_lf, vt_lf);
    DJIMotorSetRef(motor_rf, vt_rf);
    DJIMotorSetRef(motor_lb, vt_lb);
    DJIMotorSetRef(motor_rb, vt_rb);
}

/**
 * @brief 根据每个轮子的速度反馈,计算底盘的实际运动速度,逆运动解算
 *        对于双板的情况,考虑增加来自底盘板IMU的数据
 *
 */
static void No_Limit_Control()
{
    DJIMotorSetRef(motor_lf, vt_lf);
    DJIMotorSetRef(motor_rf, vt_rf);
    DJIMotorSetRef(motor_lb, vt_lb);
    DJIMotorSetRef(motor_rb, vt_rb);
}
static void EstimateSpeed()
{
    // 根据电机速度和陀螺仪的角速度进行解算,还可以利用加速度计判断是否打滑(如果有)
    // chassis_feedback_data.vx vy wz =
    //  ...
}
uint8_t UIflag = 1;
uint8_t Super_Allow_Flag;
void Super_Cap_control()
{
    if (cap->cap_msg_s.CapVot < SUPER_VOLT_MIN) {
        Super_Allow_Flag = SUPERCAP_RELAY_FLAG_FROM_USER_CLOSE;
    } else {
        Super_Allow_Flag = SUPERCAP_RELAY_FLAG_FROM_USER_OPEN;
    }

    // User允许开启电容 且 电压充足
    if (Super_flag == SUPERCAP_RELAY_FLAG_FROM_USER_OPEN && Super_Allow_Flag == SUPERCAP_RELAY_FLAG_FROM_USER_OPEN) {
        cap->cap_msg_g.power_relay_flag = SUPERCAP_RELAY_FLAG_FROM_CAN_OPEN;
    } else {
        cap->cap_msg_g.power_relay_flag = SUPERCAP_RELAY_FLAG_FROM_CAN_CLOSE;
    }

    // 物理层继电器状态改变，功率限制状态改变
    if (cap->cap_msg_s.open_flag == SUPERCAP_OPEN_FLAG_FROM_REAL_CLOSE) {
        PowerControlInit(referee_data->GameRobotState.chassis_power_limit - 25, 1);
        LimitChassisOutput();
    } else {
        PowerControlInit(250, 1);
        No_Limit_Control();
    }
}

void Power_level_get() // 获取功率裆位
{
    switch (referee_data->GameRobotState.robot_level) {
        case 1:
            cap->cap_msg_g.power_level = 1;
            break;
        case 2:
            cap->cap_msg_g.power_level = 2;
            break;
        case 3:
            cap->cap_msg_g.power_level = 3;
            break;
        case 4:
            cap->cap_msg_g.power_level = 4;
            break;
        case 5:
            cap->cap_msg_g.power_level = 5;
            break;
        case 6:
            cap->cap_msg_g.power_level = 6;
            break;
        case 7:
            cap->cap_msg_g.power_level = 7;
            break;
        case 8:
            cap->cap_msg_g.power_level = 8;
            break;
        case 9:
            cap->cap_msg_g.power_level = 9;
            break;
        case 10:
            cap->cap_msg_g.power_level = 9;
            break;
        default:
            cap->cap_msg_g.power_level = 0;
            break;
    }
    if (referee_data->GameRobotState.chassis_power_limit > 120) { // 120是最大的功率
        cap->cap_msg_g.power_level = 10;
    }
}
extern uint8_t rotate_flag;
/* 机器人底盘控制核心任务 */
void ChassisTask()
{
    // 后续增加没收到消息的处理(双板的情况)
    // 获取新的控制信息
    SubGetMessage(chassis_sub, &chassis_cmd_recv); // DEBUG

    if (chassis_cmd_recv.chassis_mode == CHASSIS_ZERO_FORCE) { // 如果出现重要模块离线或遥控器设置为急停,让电机停止
        DJIMotorStop(motor_lf);
        DJIMotorStop(motor_rf);
        DJIMotorStop(motor_lb);
        DJIMotorStop(motor_rb);
    } else { // 正常工作
        DJIMotorEnable(motor_lf);
        DJIMotorEnable(motor_rf);
        DJIMotorEnable(motor_lb);
        DJIMotorEnable(motor_rb);
    }

    // 根据控制模式设定旋转速度
    switch (chassis_cmd_recv.chassis_mode) {
        case CHASSIS_NO_FOLLOW: // 底盘不旋转,但维持全向机动,一般用于调整云台姿态
            chassis_cmd_recv.wz = 0;
            break;
        case CHASSIS_FOLLOW_GIMBAL_YAW: // 跟随云台,不单独设置pid,以误差角度平方为速度输出
            chassis_cmd_recv.wz = -chassis_cmd_recv.offset_angle * (abs(chassis_cmd_recv.offset_angle))*4;
            break;
        case CHASSIS_ROTATE: // 自旋,同时保持全向机动;当前wz维持定值,后续增加不规则的变速策略
            chassis_cmd_recv.wz = 2500;
            break;
        default:
            break;
    }
    if (rotate_flag) {
        chassis_cmd_recv.wz *= -1;
    }
    // 根据云台和底盘的角度offset将控制量映射到底盘坐标系上
    // 底盘逆时针旋转为角度正方向;云台命令的方向以云台指向的方向为x,采用右手系(x指向正北时y在正东)
    static float sin_theta, cos_theta;
    cos_theta  = arm_cos_f32(chassis_cmd_recv.offset_angle * DEGREE_2_RAD);
    sin_theta  = arm_sin_f32(chassis_cmd_recv.offset_angle * DEGREE_2_RAD);
    chassis_vx = chassis_cmd_recv.vx * cos_theta + chassis_cmd_recv.vy * sin_theta;
    chassis_vy = -chassis_cmd_recv.vx * sin_theta + chassis_cmd_recv.vy * cos_theta;
    chassis_vx *= -1;
    chassis_vy *= -1;
    // 根据控制模式进行正运动学解算,计算底盘输出
    MecanumCalculate();

    // 根据裁判系统的反馈数据和电容数据对输出限幅并设定闭环参考值
    Super_Cap_control();

    // 根据电机的反馈速度和IMU(如果有)计算真实速度
    EstimateSpeed();
    // // 获取裁判系统数据   建议将裁判系统与底盘分离，所以此处数据应使用消息中心发送
    // // 我方颜色id小于7是红色,大于7是蓝色,注意这里发送的是对方的颜色, 0:blue , 1:red
    // chassis_feedback_data.enemy_color = referee_data->GameRobotState.robot_id > 7 ? 1 : 0;
    // // 当前只做了17mm热量的数据获取,后续根据robot_def中的宏切换双枪管和英雄42mm的情况
    // chassis_feedback_data.bullet_speed = referee_data->GameRobotState.shooter_id1_17mm_speed_limit;
    // chassis_feedback_data.rest_heat = referee_data->PowerHeatData.shooter_heat0;
    Power_level_get();
    SuperCapSend(cap, (uint8_t *)&cap->cap_msg_g);
    // 推送反馈消息

    PubPushMessage(referee_pub, (void *)referee_data);
    PubPushMessage(Cap_pub, (void *)cap);
    if (referee_data->GameRobotState.robot_id != 0 && UIflag == 1) {
        get_referee_data(referee_data);
        MyUIInit();
        UIflag = 0;
    }
#ifdef ONE_BOARD
    PubPushMessage(chassis_pub, (void *)&chassis_feedback_data);
#endif
}
#ifdef CHASSIS_BOARD

#endif // DEBUG