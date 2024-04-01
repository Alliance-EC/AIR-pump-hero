// app
#include "robot_def.h"
#include "robot_cmd.h"
#include "UI_def.h"
// module
#include "remote_control.h"
#include "ins_task.h"
#include "master_process.h"
#include "message_center.h"
#include "general_def.h"
#include "super_cap.h"
#include "dji_motor.h"
// bsp
#include "bsp_dwt.h"
#include "bsp_log.h"
#include "UI.h"
// 私有宏,自动将编码器转换成角度值
static UIdate_for_change UI_data;
#define YAW_ALIGN_ANGLE     (YAW_CHASSIS_ALIGN_ECD * ECD_ANGLE_COEF_DJI) // 对齐时的角度,0-360
#define PTICH_HORIZON_ANGLE (PITCH_HORIZON_ECD * ECD_ANGLE_COEF_DJI)     // pitch水平时电机的角度,0-360

// 私有宏，用来标定当前连发还是单发
#define ONCE_FLAG    0
#define TRIGGER_FLAG 1
/* cmd应用包含的模块实例指针和交互信息存储*/
#include "can_comm.h"
#ifdef CHASSIS_BOARD
static Upboard_data data_from_upboard;
extern CANCommInstance *chassis_can_comm;
#endif                                 // DEBUG
static Publisher_t *chassis_cmd_pub;   // 底盘控制消息发布者
static Subscriber_t *chassis_feed_sub; // 底盘反馈信息订阅者

static Chassis_Ctrl_Cmd_s chassis_cmd_send;      // 发送给底盘应用的信息,包括控制信息和UI绘制相关
static Chassis_Upload_Data_s chassis_fetch_data; // 从底盘应用接收的反馈信息信息,底盘功率枪口热量与底盘运动状态等
static Chassis_Upload_Data_s chassis_send_data_ToUpboard;
static RC_ctrl_t *rc_data;              // 遥控器数据,初始化时返回
static Vision_Recv_s *vision_recv_data; // 视觉接收数据指针,初始化时返回

static Publisher_t *gimbal_cmd_pub;            // 云台控制消息发布者
static Subscriber_t *gimbal_feed_sub;          // 云台反馈信息订阅者
static Gimbal_Ctrl_Cmd_s gimbal_cmd_send;      // 传递给云台的控制信息
static Gimbal_Upload_Data_s gimbal_fetch_data; // 从云台获取的反馈信息

static Publisher_t *shoot_cmd_pub;           // 发射控制消息发布者
static Subscriber_t *shoot_feed_sub;         // 发射反馈信息订阅者
static Shoot_Ctrl_Cmd_s shoot_cmd_send;      // 传递给发射的控制信息
static Shoot_Upload_Data_s shoot_fetch_data; // 从发射获取的反馈信息

static Robot_Status_e robot_state; // 机器人整体工作状态
static Subscriber_t *Referee_Data_For_UI;
void RobotCMDInit()
{
    rc_data          = RemoteControlInit(&huart3); // 修改为对应串口,注意如果是自研板dbus协议串口需选用添加了反相器的那个 // 遥控器在底盘上 v v c
    vision_recv_data = VisionInit(&huart1);        // 视觉通信串口

    gimbal_cmd_pub      = PubRegister("gimbal_cmd", sizeof(Gimbal_Ctrl_Cmd_s));
    gimbal_feed_sub     = SubRegister("gimbal_feed", sizeof(Gimbal_Upload_Data_s));
    shoot_cmd_pub       = PubRegister("shoot_cmd", sizeof(Shoot_Ctrl_Cmd_s));
    shoot_feed_sub      = SubRegister("shoot_feed", sizeof(Shoot_Upload_Data_s));
    Referee_Data_For_UI = SubRegister("referee_data", sizeof(referee_info_t));
    // #ifdef ONE_BOARD // 双板兼容
    chassis_cmd_pub  = PubRegister("chassis_cmd", sizeof(Chassis_Ctrl_Cmd_s));
    chassis_feed_sub = SubRegister("chassis_feed", sizeof(Chassis_Upload_Data_s));
    // #endif // ONE_BOARD
    gimbal_cmd_send.pitch = 0;

    robot_state = ROBOT_READY; // 启动时机器人进入工作模式,后续加入所有应用初始化完成之后再进入
}

/**
 * @brief 根据gimbal app传回的当前电机角度计算和零位的误差
 *        单圈绝对角度的范围是0~360,说明文档中有图示
 *
 */
static void CalcOffsetAngle()
{
    // 别名angle提高可读性,不然太长了不好看,虽然基本不会动这个函数
    static float angle;
    angle = gimbal_fetch_data.yaw_motor_single_round_angle; // 从云台获取的当前yaw电机单圈角度
#if YAW_ECD_GREATER_THAN_4096                               // 如果大于180度
    if (angle > YAW_ALIGN_ANGLE && angle <= 180.0f + YAW_ALIGN_ANGLE)
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE;
    else if (angle > 180.0f + YAW_ALIGN_ANGLE)
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE - 360.0f;
    else
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE;
#else // 小于180度
    if (angle > YAW_ALIGN_ANGLE)
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE;
    else if (angle <= YAW_ALIGN_ANGLE && angle >= YAW_ALIGN_ANGLE - 180.0f)
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE;
    else
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE + 360.0f;
    if (chassis_cmd_send.offset_angle > 180)
        chassis_cmd_send.offset_angle = chassis_cmd_send.offset_angle - 360.0f;
#endif
}

/**
 * @brief 控制输入为遥控器(调试时)的模式和控制量设置
 *
 */

#ifndef TESTCODE
static void RemoteControlSet()
{
    // 控制底盘和云台运行模式,云台待添加,云台是否始终使用IMU数据?
    if (switch_is_down(rc_data[TEMP].rc.switch_right)) // 右侧开关状态[下],底盘跟随云台
    {
        chassis_cmd_send.chassis_mode = CHASSIS_ROTATE;
        gimbal_cmd_send.gimbal_mode   = GIMBAL_GYRO_MODE;
    } else if (switch_is_mid(rc_data[TEMP].rc.switch_right)) // 右侧开关状态[中],底盘和云台分离,底盘保持不转动
    {
        chassis_cmd_send.chassis_mode = CHASSIS_NO_FOLLOW;
        gimbal_cmd_send.gimbal_mode   = GIMBAL_FREE_MODE;
    }

    // 云台参数,确定云台控制数据
    if (switch_is_mid(rc_data[TEMP].rc.switch_left)) // 左侧开关状态为[中],视觉模式
    {
        // 待添加,视觉会发来和目标的误差,同样将其转化为total angle的增量进行控制
        // ...
    }
    // 左侧开关状态为[下],或视觉未识别到目标,纯遥控器拨杆控制
    if (switch_is_down(rc_data[TEMP].rc.switch_left) /*|| vision_recv_data->target_state == NO_TARGET*/) { // 按照摇杆的输出大小进行角度增量,增益系数需调整
        gimbal_cmd_send.yaw   = 0.005f * (float)rc_data[TEMP].rc.rocker_l_;
        gimbal_cmd_send.pitch = 0.0001f * (float)rc_data[TEMP].rc.rocker_l1;
    }
    // 云台软件限位

    // 底盘参数,目前没有加入小陀螺(调试似乎暂时没有必要),系数需要调整
    chassis_cmd_send.vx = 10.0f * (float)rc_data[TEMP].rc.rocker_r_; // _水平方向
    chassis_cmd_send.vy = 10.0f * (float)rc_data[TEMP].rc.rocker_r1; // _竖直方向

    // 发射参数
    // if (switch_is_up(rc_data[TEMP].rc.switch_right)) // 右侧开关状态[上],弹舱打开
    //     ;                                            // 弹舱舵机控制,待添加servo_motor模块,开启
    // else
    //     ; // 弹舱舵机控制,待添加servo_motor模块,关闭

    // 摩擦轮控制,拨轮向上打为负,向下为正
    if (rc_data[TEMP].rc.dial < -100) // 向上超过100,打开摩擦轮
        shoot_cmd_send.friction_mode = FRICTION_ON;
    else
        shoot_cmd_send.friction_mode = FRICTION_OFF;
    // 拨弹控制,遥控器固定为一种拨弹模式,可自行选择
    if (rc_data[TEMP].rc.dial < -500)
        shoot_cmd_send.load_mode = LOAD_BURSTFIRE;
    else
        shoot_cmd_send.load_mode = LOAD_STOP;
    // 射频控制,固定每秒1发,后续可以根据左侧拨轮的值大小切换射频,
    shoot_cmd_send.shoot_rate = 8;
}

#endif

#ifdef TESTCODE
static uint8_t UI_flag        = 1;
static uint8_t One_shoot_flag = 1;
uint8_t Super_flag            = 0;
static void MouseKeySet()
{
    if ((rc_data[TEMP].rc.switch_left == RC_SW_DOWN) && (rc_data[TEMP].rc.switch_right == RC_SW_DOWN)) {
        chassis_cmd_send.chassis_mode = CHASSIS_ZERO_FORCE;
        gimbal_cmd_send.gimbal_mode   = GIMBAL_ZERO_FORCE;
        shoot_cmd_send.shoot_mode     = SHOOT_OFF;
        shoot_cmd_send.friction_mode  = FRICTION_OFF;
        shoot_cmd_send.load_mode      = LOAD_STOP;
    }
    chassis_cmd_send.vy = rc_data[TEMP].key[KEY_PRESS].w * 80000 - rc_data[TEMP].key[KEY_PRESS].s * 80000; // 系数待测
    chassis_cmd_send.vx = rc_data[TEMP].key[KEY_PRESS].a * 80000 - rc_data[TEMP].key[KEY_PRESS].d * 80000;

    gimbal_cmd_send.yaw   = (float)rc_data[TEMP].mouse.x / 660 * 1; // 系数待测
    gimbal_cmd_send.pitch = -(float)rc_data[TEMP].mouse.y / 660 * 20;
    if (rc_data[TEMP].key_count[KEY_PRESS][Key_B] % 2 == 1) {
        if (UI_flag == 1) {
            MyUIInit();
            UI_flag = 0;
        }
    } else
        UI_flag = 1;
    if (rc_data[TEMP].key[KEY_PRESS].ctrl == 1) {
        gimbal_cmd_send.yaw /= 3;
        gimbal_cmd_send.pitch /= 3;
    }
    if (rc_data[TEMP].key[KEY_PRESS].shift == 1) {
        Super_flag = 1;
    } else
        Super_flag = 0;
    if (rc_data[TEMP].key_count[KEY_PRESS][Key_R] % 2 == 0) {
        One_shoot_flag = 1;
    } else {
        One_shoot_flag = 0;
    }
    switch (rc_data[TEMP].key_count[KEY_PRESS_WITH_CTRL][Key_V] % 2) // V键开启摩擦轮
    {
        case 1:
            if (shoot_cmd_send.friction_mode != FRICTION_ON) {
                shoot_cmd_send.friction_mode = FRICTION_ON;
            }
            break;
        case 0:
            if (shoot_cmd_send.friction_mode == FRICTION_ON) {
                shoot_cmd_send.friction_mode = FRICTION_OFF;
            }
            break;
    }
    switch (rc_data[TEMP].key_count[KEY_PRESS_WITH_CTRL][Key_C] % 2) // C键开启陀螺模式
    {
        case 1:
            if (chassis_cmd_send.chassis_mode != CHASSIS_ROTATE) {
                chassis_cmd_send.chassis_mode = CHASSIS_ROTATE;
            }
            break;
        case 0:
            if (chassis_cmd_send.chassis_mode == CHASSIS_ROTATE) {
                chassis_cmd_send.chassis_mode = CHASSIS_FOLLOW_GIMBAL_YAW;
            }
            break;
    }
    switch (rc_data[TEMP].key[KEY_PRESS].shift) // 待添加 按shift允许超功率 消耗缓冲能量
    {
        case 1:

            break;

        default:

            break;
    }
    switch (rc_data[TEMP].mouse.press_l) {
        case 1:
            if (shoot_cmd_send.friction_mode == FRICTION_ON) {
                if (One_shoot_flag == 1) {
                    shoot_cmd_send.load_mode = LOAD_1_BULLET;
                } else
                    shoot_cmd_send.load_mode = LOAD_BURSTFIRE;
            }
            break;
        case 0:
            shoot_cmd_send.load_mode = LOAD_MODE;
            break;
    }
}

static void RemoteControlSet()
{
    if ((rc_data[TEMP].rc.switch_left == RC_SW_DOWN) && (rc_data[TEMP].rc.switch_right == RC_SW_DOWN)) {

        chassis_cmd_send.chassis_mode = CHASSIS_ZERO_FORCE;
        gimbal_cmd_send.gimbal_mode   = GIMBAL_ZERO_FORCE;
        shoot_cmd_send.friction_mode  = FRICTION_OFF;
        shoot_cmd_send.load_mode      = LOAD_STOP; // 所有电机停止工作
    } else {

        // 云台底盘命令
        gimbal_cmd_send.yaw   = 0.00004f * (float)rc_data[TEMP].rc.rocker_l_;
        gimbal_cmd_send.pitch = 0.001f * (float)rc_data[TEMP].rc.rocker_l1;
        chassis_cmd_send.vx   = 50.0f * (float)rc_data[TEMP].rc.rocker_r_; // Chassis_水平方向
        chassis_cmd_send.vy   = 50.0f * (float)rc_data[TEMP].rc.rocker_r1; // Chassis_竖直方向

        gimbal_cmd_send.gimbal_mode = GIMBAL_FREE_MODE; // 云台只有两个模式，或停止，或FREE

        switch (rc_data[TEMP].rc.switch_right) {
            case RC_SW_DOWN:
                chassis_cmd_send.chassis_mode = CHASSIS_ROTATE; // 右下，左不下，底盘小陀螺 【发射时不进行小陀螺】
                break;
            case RC_SW_MID:
                chassis_cmd_send.chassis_mode = CHASSIS_NO_FOLLOW; // 右中，云台FREE，底盘不随动
                break;
            case RC_SW_UP:
                chassis_cmd_send.chassis_mode = CHASSIS_FOLLOW_GIMBAL_YAW; // 右上，底盘随动
                break;
            default:

                break;
        }

        // 发射机构命令
        static int flag_load         = 0;
        static int flag_friction     = 0;
        static int flag_once_or_trig = 0;
        static int flag_shoot_change = 0;

        switch (rc_data[TEMP].rc.switch_left) {
            case RC_SW_DOWN:

                if (flag_load == 1) {
                    //     if (flag_once_or_trig == TRIGGER_FLAG)
                    //         shoot_cmd_send.load_mode = LOAD_BURSTFIRE; // 拨弹盘持续转动
                    //     if (flag_once_or_trig == ONCE_FLAG)
                    shoot_cmd_send.load_mode = LOAD_1_BULLET;
                } else {
                    shoot_cmd_send.load_mode = LOAD_MODE;
                }

                break;
            case RC_SW_MID:

                if ((shoot_cmd_send.load_mode == LOAD_BURSTFIRE) || (shoot_cmd_send.load_mode == LOAD_1_BULLET)) {
                    flag_load                = 1;
                    shoot_cmd_send.load_mode = LOAD_MODE;
                }
                if (flag_load == 0) {
                    flag_load = 1;
                }
                if (shoot_cmd_send.friction_mode == FRICTION_ON) {
                    flag_friction = 0;
                } else {
                    flag_friction = 1;
                }
                if (shoot_cmd_send.shoot_mode == SHOOT_OFF) {
                    shoot_cmd_send.shoot_mode = SHOOT_ON;
                }

                break;
            case RC_SW_UP:

                if (flag_friction == 1) {
                    shoot_cmd_send.friction_mode = FRICTION_ON; // 摩擦轮持续转动
                } else {
                    shoot_cmd_send.friction_mode = FRICTION_OFF;
                }

                break;
            default:
                break;
        }
        // if ((rc_data[TEMP].rc.dial < -300) && (flag_shoot_change == 0)) {
        //     if (flag_once_or_trig == ONCE_FLAG) {
        //         flag_once_or_trig = TRIGGER_FLAG;
        //     } else if (flag_once_or_trig == TRIGGER_FLAG) {
        //         flag_once_or_trig = ONCE_FLAG;
        //     }
        //     flag_shoot_change = 1;
        // }

        if (shoot_cmd_send.friction_mode == FRICTION_OFF) {
            shoot_cmd_send.load_mode = LOAD_STOP;
        } // 阻止不发弹而装弹
    }
}

#endif

/**
 * @brief 输入为键鼠时模式和控制量设置
 *
 */
/* 机器人核心控制任务,200Hz频率运行(必须高于视觉发送频率) */
static referee_info_t referee_data;
void Get_UI_Data() // 将裁判系统数据和机器人状态传入UI
{
    SubGetMessage(Referee_Data_For_UI, &referee_data);
    UI_data.fir_mode  = shoot_cmd_send.friction_mode;
    UI_data.rot_mode  = chassis_cmd_send.chassis_mode;
    UI_data.remain_HP = referee_data.GameRobotState.remain_HP;
    UI_data.load_Mode = One_shoot_flag;
    UI_data.Max_HP    = referee_data.GameRobotState.max_HP;
}

void RobotCMDTask()
{
    // 从其他应用获取回传数据
#ifdef ONEBROAD
    SubGetMessage(shoot_feed_sub, &shoot_fetch_data);
    SubGetMessage(gimbal_feed_sub, &gimbal_fetch_data);
#endif // DEBUG
#ifdef CHASSIS_BOARD
    data_from_upboard = *(Upboard_data *)CANCommGet(chassis_can_comm);
    shoot_fetch_data  = data_from_upboard.Shoot_data;
    gimbal_fetch_data = data_from_upboard.Gimbal_data;
#endif // DEBUG
    SubGetMessage(chassis_feed_sub, (void *)&chassis_fetch_data);
    CalcOffsetAngle();

#ifndef TESTCODE
    // 根据遥控器左侧开关,确定当前使用的控制模式为遥控器调试还是键鼠
    if (switch_is_down(rc_data[TEMP].rc.switch_left)) // 遥控器左侧开关状态为[下],遥控器控制
        RemoteControlSet();
    else if (switch_is_up(rc_data[TEMP].rc.switch_left)) // 遥控器左侧开关状态为[上],键盘控制
        MouseKeySet();

        // EmergencyHandler(); // 处理模块离线和遥控器急停等紧急情况

        // 设置视觉发送数据,还需增加加速度和角速度数据
        // VisionSetFlag(chassis_fetch_data.enemy_color,,chassis_fetch_data.bullet_speed)

        // 推送消息,双板通信,视觉通信等
        // 其他应用所需的控制数据在remotecontrolsetmode和mousekeysetmode中完成设置
#ifdef ONE_BOARD
    PubPushMessage(chassis_cmd_pub, (void *)&chassis_cmd_send);
    PubPushMessage(shoot_cmd_pub, (void *)&shoot_cmd_send);
    PubPushMessage(gimbal_cmd_pub, (void *)&gimbal_cmd_send);
    VisionSend(&vision_send_data);
#endif // ONE_BOARD
#endif
#ifdef TESTCODE

    RemoteControlSet();
    if ((switch_is_mid(rc_data[TEMP].rc.switch_left)) && (switch_is_up(rc_data[TEMP].rc.switch_right))) {
        MouseKeySet();
    }
#endif

#ifdef CHASSIS_BOARD
    chassis_send_data_ToUpboard.gimbal_cmd_upload = gimbal_cmd_send;
    chassis_send_data_ToUpboard.shoot_cmd_upload  = shoot_cmd_send;
    CANCommSend(chassis_can_comm, (void *)&chassis_send_data_ToUpboard);
    PubPushMessage(chassis_cmd_pub, (void *)&chassis_cmd_send);
    Get_UI_Data();
#endif // DEBUG
}
#if defined(CHASSIS_BOARD) || defined(ONE_BOARD)
void UItask(void *argument)
{
    /* USER CODE BEGIN UItask */
    /* Infinite loop */
    for (;;) {
        if (check_to_change_UI(&UI_data) == 1) {
            MyUIRefresh();
        }
        UIfresh_num();
        osDelay(40);
    }
    /* USER CODE END UItask */
}
#endif