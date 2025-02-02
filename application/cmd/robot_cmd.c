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
#include "buzzer.h"
#include "power_calc.h"
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
static RC_ctrl_t *rc_data; // 遥控器数据,初始化时返回

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
static Subscriber_t *Cap_data_For_UI;
static SuperCapInstance Cap;
void RobotCMDInit()
{
    rc_data             = RemoteControlInit(&huart3); // 修改为对应串口,注意如果是自研板dbus协议串口需选用添加了反相器的那个 // 遥控器在底盘上 v v c
    gimbal_cmd_pub      = PubRegister("gimbal_cmd", sizeof(Gimbal_Ctrl_Cmd_s));
    gimbal_feed_sub     = SubRegister("gimbal_feed", sizeof(Gimbal_Upload_Data_s));
    shoot_cmd_pub       = PubRegister("shoot_cmd", sizeof(Shoot_Ctrl_Cmd_s));
    shoot_feed_sub      = SubRegister("shoot_feed", sizeof(Shoot_Upload_Data_s));
    Referee_Data_For_UI = SubRegister("referee_data", sizeof(referee_info_t));
    Cap_data_For_UI     = SubRegister("cap_data", sizeof(SuperCapInstance));
    // #ifdef ONE_BOARD // 双板兼容
    chassis_cmd_pub  = PubRegister("chassis_cmd", sizeof(Chassis_Ctrl_Cmd_s));
    chassis_feed_sub = SubRegister("chassis_feed", sizeof(Chassis_Upload_Data_s));
    // #endif // ONE_BOARD
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

static uint8_t UI_flag        = 1;
static uint8_t One_shoot_flag = 1;
uint8_t Super_flag            = 0;
static uint8_t last_count_F;
static uint8_t last_count_C;
static void MouseKeySet()
{
    if ((rc_data[TEMP].rc.switch_left == RC_SW_DOWN) && (rc_data[TEMP].rc.switch_right == RC_SW_DOWN)) {
        chassis_cmd_send.chassis_mode = CHASSIS_ZERO_FORCE;
        gimbal_cmd_send.gimbal_mode   = GIMBAL_ZERO_FORCE;
        shoot_cmd_send.shoot_mode     = SHOOT_OFF;
        shoot_cmd_send.load_mode      = LOAD_STOP;
    }
    gimbal_cmd_send.gimbal_mode = GIMBAL_GYRO_MODE;
    chassis_cmd_send.vy         = rc_data[TEMP].key[KEY_PRESS].w * 26400 - rc_data[TEMP].key[KEY_PRESS].s * 26400;
    chassis_cmd_send.vx         = rc_data[TEMP].key[KEY_PRESS].a * 26400 - rc_data[TEMP].key[KEY_PRESS].d * 26400;

    gimbal_cmd_send.yaw   = (float)rc_data[TEMP].mouse.x / 660 * 0.5;
    gimbal_cmd_send.pitch = -(float)rc_data[TEMP].mouse.y / 660 * 10;
    if (rc_data[TEMP].mouse.press_l == 1) {
        shoot_cmd_send.air_pump_mode = AIR_PUMP_ON; // 左键开火，有单发限制
    } else
        shoot_cmd_send.air_pump_mode = AIR_PUMP_OFF;
    if (rc_data[TEMP].mouse.press_r == 1) {
        gimbal_cmd_send.vision_mode = VISION_ON;
    } else
        gimbal_cmd_send.vision_mode = VISION_OFF;             // 右键开启自瞄
    if (rc_data[TEMP].key_count[KEY_PRESS][Key_B] % 2 == 1) { // UI刷新
        if (UI_flag == 1) {
            MyUIInit();
            UI_flag = 0;
        }
    } else
        UI_flag = 1;
    if (rc_data[TEMP].key[KEY_PRESS].ctrl == 1) { // CTRL减慢云台转速
        gimbal_cmd_send.yaw /= 3;
        gimbal_cmd_send.pitch /= 3;
    }

    if (rc_data[TEMP].key[KEY_PRESS].shift == 1) { // 超电
        Super_flag = 1;
    } else
        Super_flag = 0;

    switch (rc_data[TEMP].key_count[KEY_PRESS_WITH_CTRL][Key_V] % 2) // V键开启摩擦轮(气动等效摩擦轮)
    {
        case 1:
            shoot_cmd_send.shoot_mode = SHOOT_ON;
            break;
        case 0:
            shoot_cmd_send.shoot_mode = SHOOT_OFF;
            break;
    }

    switch (rc_data[TEMP].key_count[KEY_PRESS][Key_Q] % 2) { // Q键打开望远镜
        case 1:
            gimbal_cmd_send.sight_mode = SIGHT_ON;
            break;
        case 0:
            gimbal_cmd_send.sight_mode = SIGHT_OFF;
            break;
    }
     switch (rc_data[TEMP].key_count[KEY_PRESS][Key_Q] % 2) { // Q键打开望远镜
        case 1:
            gimbal_cmd_send.sight_mode = SIGHT_ON;
            break;
        case 0:
            gimbal_cmd_send.sight_mode = SIGHT_OFF;
            break;
    }
    switch (rc_data[TEMP].key_count[KEY_PRESS][Key_E] % 3) { // E键更改图传模式
        case 1:
            gimbal_cmd_send.image_mode = snipe;
            break;
        case 0:
            gimbal_cmd_send.image_mode = Follow_shoot;
            break;
        case 2:
        gimbal_cmd_send.image_mode = sentry;
        break;
    }
    if (last_count_F != rc_data[TEMP].key_count[KEY_PRESS][Key_F]) // 云台自由模式
    {
        if (chassis_cmd_send.chassis_mode == CHASSIS_NO_FOLLOW) {
            chassis_cmd_send.chassis_mode = CHASSIS_FOLLOW_GIMBAL_YAW;
        } else {
            chassis_cmd_send.chassis_mode = CHASSIS_NO_FOLLOW;
        }
    }

    if (last_count_C != rc_data[TEMP].key_count[KEY_PRESS][Key_C]) // 小陀螺
    {
        if (chassis_cmd_send.chassis_mode == CHASSIS_ROTATE) {
            chassis_cmd_send.chassis_mode = CHASSIS_FOLLOW_GIMBAL_YAW;
        } else {
            chassis_cmd_send.chassis_mode = CHASSIS_ROTATE;
        }
    }
    last_count_C = rc_data[TEMP].key_count[KEY_PRESS][Key_C];
    last_count_F = rc_data[TEMP].key_count[KEY_PRESS][Key_F];
}
static int8_t start_flag;
static int8_t air_flag, loader_flag;
uint8_t rotate_flag;
static void RemoteControlSet()
{
    if ((rc_data[TEMP].rc.switch_left == RC_SW_DOWN) && (rc_data[TEMP].rc.switch_right == RC_SW_DOWN)) {

        chassis_cmd_send.chassis_mode = CHASSIS_ZERO_FORCE;
        gimbal_cmd_send.gimbal_mode   = GIMBAL_ZERO_FORCE;
        shoot_cmd_send.shoot_mode     = SHOOT_OFF;
        shoot_cmd_send.load_mode      = LOAD_STOP; // 所有电机停止工作
    } else {

        // 云台底盘命令
        gimbal_cmd_send.yaw   = 0.0001f * (float)rc_data[TEMP].rc.rocker_l_;
        gimbal_cmd_send.pitch = 0.002f * (float)rc_data[TEMP].rc.rocker_l1;
        chassis_cmd_send.vx   = 40.0f * (float)rc_data[TEMP].rc.rocker_r_; // Chassis_水平方向
        chassis_cmd_send.vy   = 40.0f * (float)rc_data[TEMP].rc.rocker_r1; // Chassis_竖直方向

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

        if (rc_data[TEMP].rc.dial < -330) {
            gimbal_cmd_send.vision_mode = VISION_ON;
        } else {
            gimbal_cmd_send.vision_mode = VISION_OFF;
        } // 拨轮打开自瞄
        if(rc_data[TEMP].rc.switch_left==RC_SW_UP)
        rotate_flag=1;
        if(rc_data[TEMP].rc.switch_left==RC_SW_MID) rotate_flag=0;
        gimbal_cmd_send.gimbal_mode = GIMBAL_GYRO_MODE;
        // 发射机构命令
        switch (rc_data[TEMP].rc.switch_left) {
            case RC_SW_UP:
                if (start_flag == 1) {
                    if (shoot_cmd_send.shoot_mode == SHOOT_OFF)
                        shoot_cmd_send.shoot_mode = SHOOT_ON;
                    else {
                        shoot_cmd_send.shoot_mode = SHOOT_OFF;
                    }
                    start_flag = 0;
                }
                break;
            case RC_SW_MID:
                if (loader_flag == 1) {
                    shoot_cmd_send.load_mode = LOAD_ON;
                    loader_flag              = 0;
                }
                start_flag                   = 1;
                air_flag                     = 1;
                shoot_cmd_send.air_pump_mode = AIR_PUMP_OFF;
                break;
            case RC_SW_DOWN:
                if (air_flag == 1) {
                    shoot_cmd_send.air_pump_mode = AIR_PUMP_ON;
                    air_flag                     = 0;
                }
                loader_flag              = 1;
                shoot_cmd_send.load_mode = LOAD_STOP;
                break;
        }
    }
}
/**
 * @brief 输入为键鼠时模式和控制量设置
 *
 */
/* 机器人核心控制任务,200Hz频率运行(必须高于视觉发送频率) */
static referee_info_t referee_data;

void Get_UI_Data() // 将裁判系统数据和机器人状态传入UI
{
    UI_data.Bullet_ready = shoot_fetch_data.bullet_ready;
    UI_data.All_robot_HP = referee_data.GameRobotHP;
    UI_data.pitch_data   = gimbal_fetch_data.Pitch_data;
    UI_data.shoot_mode   = shoot_cmd_send.shoot_mode;
    UI_data.chassis_mode = chassis_cmd_send.chassis_mode;
    UI_data.remain_HP    = referee_data.GameRobotState.remain_HP;
    UI_data.load_Mode    = One_shoot_flag;
    UI_data.Max_HP       = referee_data.GameRobotState.max_HP;
    UI_data.Angle        = chassis_cmd_send.offset_angle;
    UI_data.CapVot       = Cap.cap_msg_s.CapVot;
    UI_data.Air_ready    = shoot_fetch_data.bullet_ready;
    UI_data.vision_servo = gimbal_cmd_send.image_mode;
    if(referee_data.referee_id.Robot_Color>7)
    UI_data.outpost_HP = referee_data.GameRobotHP.red_outpost_HP;
    else UI_data.outpost_HP = referee_data.GameRobotHP.blue_base_HP;
}
extern Power_Data_s  power_data;
extern DJIMotorInstance *motor_lf, *motor_rf, *motor_lb, *motor_rb;
void RobotCMDTask()
{
    // 从其他应用获取回传数据
#ifdef ONE_BOARD
    SubGetMessage(shoot_feed_sub, &shoot_fetch_data);
    SubGetMessage(gimbal_feed_sub, &gimbal_fetch_data);
#endif // DEBUG
#ifdef CHASSIS_BOARD
    data_from_upboard = *(Upboard_data *)CANCommGet(chassis_can_comm);
    shoot_fetch_data  = data_from_upboard.Shoot_data;
    gimbal_fetch_data = data_from_upboard.Gimbal_data;
#endif // DEBUG
    SubGetMessage(chassis_feed_sub, (void *)&chassis_fetch_data);
    SubGetMessage(Referee_Data_For_UI, (void *)&referee_data);
    SubGetMessage(Cap_data_For_UI, (void *)&Cap);
    CalcOffsetAngle();
    if ((switch_is_mid(rc_data[TEMP].rc.switch_left)) && (switch_is_up(rc_data[TEMP].rc.switch_right))) {
        MouseKeySet();
    } else
        RemoteControlSet();

    PubPushMessage(chassis_cmd_pub, (void *)&chassis_cmd_send);
    Get_UI_Data();

#ifdef CHASSIS_BOARD
    // chassis_send_data_ToUpboard.current1=motor_lf->motor_controller.speed_PID.Output;
    // chassis_send_data_ToUpboard.current2=motor_rf->motor_controller.speed_PID.Output;
    // chassis_send_data_ToUpboard.current3=motor_lb->motor_controller.speed_PID.Output;
    // chassis_send_data_ToUpboard.current4=motor_rb->motor_controller.speed_PID.Output;
    // chassis_send_data_ToUpboard.speed_aps=motor_rb->measure.speed_aps;
    // chassis_send_data_ToUpboard.pre_power=power_data.total_power;
    // chassis_send_data_ToUpboard.real_power=referee_data.PowerHeatData.chassis_power;
    chassis_send_data_ToUpboard.gimbal_cmd_upload = gimbal_cmd_send;
    chassis_send_data_ToUpboard.shoot_cmd_upload  = shoot_cmd_send;
    if(referee_data.GameRobotState.mains_power_shooter_output==0)
    chassis_send_data_ToUpboard.shoot_cmd_upload.Shoot_power=POWER_OFF;
    else chassis_send_data_ToUpboard.shoot_cmd_upload.Shoot_power=POWER_ON;
    CANCommSend(chassis_can_comm, (void *)&chassis_send_data_ToUpboard);
#endif // DEBUG

#ifdef ONE_BOARD
    PubPushMessage(chassis_cmd_pub, (void *)&chassis_cmd_send);
    PubPushMessage(shoot_cmd_pub, (void *)&shoot_cmd_send);
    PubPushMessage(gimbal_cmd_pub, (void *)&gimbal_cmd_send);
    // VisionSend(&vision_send_data);
#endif // ONE_BOARD
}
#if defined(CHASSIS_BOARD) || defined(ONE_BOARD)
void UItask(void *argument)
{
    /* USER CODE BEGIN UItask */
    /* Infinite loop */
    for (;;) {
        check_to_change_UI(&UI_data);
        MyUIRefresh();
        UIfresh_Always();
        osDelay(40);
    }
    /* USER CODE END UItask */
}
#endif
__attribute__((noreturn)) void BuzzerTask(void *argument)
{
    UNUSED(argument);
    while (1) {
        if (shoot_cmd_send.shoot_mode == SHOOT_ON) {
            buzzer_one_note(Si_freq, 0.5);
            buzzer_one_note(So_freq, 0.5);
            DWT_Delay(0.5);
        }
        osDelay(1);
    }
}