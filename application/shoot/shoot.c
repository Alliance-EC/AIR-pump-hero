#include "shoot.h"
#include "motor_def.h"
#include "robot_def.h"

#include "dji_motor.h"
#include "message_center.h"
#include "bsp_dwt.h"
#include "general_def.h"
#include "user_lib.h"
#include <stdint.h>

/* 对于双发射机构的机器人,将下面的数据封装成结构体即可,生成两份shoot应用实例 */
static DJIMotorInstance *friction_l, *friction_r, *loader; // 拨盘电机
// static servo_instance *lid; 需要增加弹舱盖

static Publisher_t *shoot_pub;
Shoot_Ctrl_Cmd_s shoot_cmd_recv; // 来自cmd的发射控制信息
static Subscriber_t *shoot_sub;
Shoot_Upload_Data_s shoot_feedback_data; // 来自cmd的发射控制信息
uint8_t Shoot_limit_for_oneshootPC6;
uint8_t Shoot_limit_for_oneshootPE14;
uint8_t One_Shoot_flag;
#ifdef SAMPLING
uint8_t *flag_complete;
float *flag_frequnency_now;
uint8_t store_flag_complete;
float store_flag_frequnency_now;
float sampling_result;
#endif

// dwt定时,计算冷却用
#ifndef SAMPLING
static float hibernate_time = 0, dead_time = 0;
#endif

void ShootInit()
{
    // 左摩擦轮
    Motor_Init_Config_s friction_config = {
        .can_init_config = {
            .can_handle = &hcan1,
        },
        .controller_param_init_config = {
            .speed_PID = {
                .Kp            = 20, // 20
                .Ki            = 1,  // 1
                .Kd            = 0,
                .Improve       = PID_Integral_Limit,
                .IntegralLimit = 10000,
                .MaxOut        = 15000,
            },
            .current_PID = {
                .Kp            = 1.0, // 0.7
                .Ki            = 0.1, // 0.1
                .Kd            = 0,
                .Improve       = PID_Integral_Limit,
                .IntegralLimit = 10000,
                .MaxOut        = 15000,
            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,

            .outer_loop_type    = SPEED_LOOP,
            .close_loop_type    = SPEED_LOOP | CURRENT_LOOP,
            .motor_reverse_flag = MOTOR_DIRECTION_REVERSE,
        },
        .motor_type = M3508};
    friction_config.can_init_config.tx_id = 7;
    friction_l                            = DJIMotorInit(&friction_config);

    friction_config.can_init_config.tx_id                             = 8; // 右摩擦轮,改txid和方向就行
    friction_config.controller_setting_init_config.motor_reverse_flag = MOTOR_DIRECTION_NORMAL;
    friction_r                                                        = DJIMotorInit(&friction_config);

    // 拨盘电机
    Motor_Init_Config_s loader_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id      = 6,
        },
        .controller_param_init_config = {
            .angle_PID = {
                // 如果启用位置环来控制发弹,需要较大的I值保证输出力矩的线性度否则出现接近拨出的力矩大幅下降
                .Kp     = 20, // 10
                .Ki     = 0,
                .Kd     = 0,
                .MaxOut = 2000,
            },
            .speed_PID = {
                .Kp            = 4,
                .Ki            = 4.21281502722044,
                .Kd            = 0,
                .Improve       = PID_Integral_Limit,
                .IntegralLimit = 5000,
                .MaxOut        = 20000,
            },
            .current_PID = {
                .Kp            = 3.21281502722044,
                .Ki            = 0,
                .Kd            = 0,
                .Improve       = PID_Integral_Limit,
                .IntegralLimit = 5000,
                .MaxOut        = 5000,
            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED, .speed_feedback_source = MOTOR_FEED,
            .outer_loop_type    = SPEED_LOOP, // 初始化成SPEED_LOOP,让拨盘停在原地,防止拨盘上电时乱转
            .close_loop_type    = SPEED_LOOP | ANGLE_LOOP,
            .motor_reverse_flag = MOTOR_DIRECTION_REVERSE, // 注意方向设置为拨盘的拨出的击发方向
        },
        .motor_type = M3508 // 英雄使用m3508
    };
    loader = DJIMotorInit(&loader_config);

    shoot_pub = PubRegister("shoot_feed", sizeof(Shoot_Upload_Data_s));
    shoot_sub = SubRegister("shoot_cmd", sizeof(Shoot_Ctrl_Cmd_s));
}
static float Last_verb_Of_load;
static uint8_t block_flag = 0;
static int friction_speed=35000;
void block_shook_check(float Now_verb_Of_load) // 堵转检测函数
{
    if (
        fabs(Now_verb_Of_load) <= (loader)->motor_controller.pid_ref / 200 && fabs(Last_verb_Of_load) <= (loader)->motor_controller.pid_ref / 200 && (loader)->motor_controller.pid_ref != 0) {
        block_flag = 1;
    } else
        block_flag = 0;
    Last_verb_Of_load = Now_verb_Of_load;
}
static float last_bullet_speed;
void Get_New_friction_speed(int Now_bullet_speed)
{
    if(Now_bullet_speed>14.5)
    {
        friction_speed+=(Target_bullet_speed-Now_bullet_speed)*100.0f  ;
    }
}

/* 机器人发射机构控制核心任务 */
void ShootTask()
{

    // 从cmd获取控制数据
#ifdef ONE_BROAD
    SubGetMessage(shoot_sub, &shoot_cmd_recv);
#endif // ONE_BORAD
#ifdef GIMBAL_BROAD
// 在robot.c可以获得值
#endif // DEBUG
    // 对shoot mode等于SHOOT_STOP的情况特殊处理,直接停止所有电机(紧急停止)
    if (shoot_cmd_recv.shoot_mode == SHOOT_OFF) {
        DJIMotorStop(friction_l);
        DJIMotorStop(friction_r);
        DJIMotorStop(loader);
    } else // 恢复运行
    {
        DJIMotorEnable(friction_l);
        DJIMotorEnable(friction_r);
        DJIMotorEnable(loader);
    }

    // 如果上一次触发单发或3发指令的时间加上不应期仍然大于当前时间(尚未休眠完毕),直接返回即可
    // 单发模式主要提供给能量机关激活使用(以及英雄的射击大部分处于单发)
    // if (hibernate_time + dead_time > DWT_GetTimeline_ms())
    //     return;

    // 若不在休眠状态,根据robotCMD传来的控制模式进行拨盘电机参考值设定和模式切换
    if (Shoot_limit_for_oneshootPC6 == 1) {
        One_Shoot_flag = 0;
    }

#ifndef SAMPLING
    block_shook_check((loader)->measure.speed_aps);
    if (block_flag == 1) {
        DJIMotorOuterLoop(loader, ANGLE_LOOP);
        DJIMotorSetRef(loader, loader->measure.total_angle + ONE_BULLET_DELTA_ANGLE * 9);
        DWT_Delay(0.2);
        block_flag = 0;
    }
    switch (shoot_cmd_recv.load_mode) {
        // 停止拨盘
        case LOAD_STOP:
            DJIMotorOuterLoop(loader, SPEED_LOOP); // 切换到速度环
            DJIMotorSetRef(loader, 0);             // 同时设定参考值为0,这样停止的速度最快
            break;
        // 单发模式,根据鼠标按下的时间,触发一次之后需要进入不响应输入的状态(否则按下的时间内可能多次进入,导致多次发射)
        case LOAD_1_BULLET:
            if (One_Shoot_flag == 1 && block_flag == 0 && shoot_cmd_recv.friction_mode == FRICTION_ON) {
                DJIMotorOuterLoop(loader, SPEED_LOOP);
                DJIMotorSetRef(loader, 12000); // 完成1发弹丸发射的时间
            } else {
                DJIMotorSetRef(loader, 0);
            }

            break;
        // 三连发,如果不需要后续可能删除
        case LOAD_3_BULLET:
            DJIMotorOuterLoop(loader, ANGLE_LOOP);                                             // 切换到速度环
            DJIMotorSetRef(loader, loader->measure.total_angle + 19 * ONE_BULLET_DELTA_ANGLE); // 增加3发
            hibernate_time = DWT_GetTimeline_ms();                                             // 记录触发指令的时间
            dead_time      = 300;                                                              // 完成3发弹丸发射的时间
            break;
        // 连发模式,对速度闭环,射频后续修改为可变,目前固定为1Hz
        case LOAD_BURSTFIRE:
            if (shoot_cmd_recv.friction_mode == FRICTION_ON) {
                DJIMotorOuterLoop(loader, SPEED_LOOP);
                DJIMotorSetRef(loader, 12000);
            }
            break;
        // 拨盘反转,对速度闭环,后续增加卡弹检测(通过裁判系统剩余热量反馈和电机电流)
        // 也有可能需要从switch-case中独立出来
        case LOAD_MODE: // 装弹模式
            if (Shoot_limit_for_oneshootPC6 == 1 && shoot_cmd_recv.friction_mode == FRICTION_ON && block_flag == 0) {
                DJIMotorOuterLoop(loader, SPEED_LOOP);
                DJIMotorSetRef(loader, 8000); // 完成1发弹丸发射的时间
            } else {
                One_Shoot_flag = 1;
                DJIMotorOuterLoop(loader, SPEED_LOOP); // 切换到速度环
                DJIMotorSetRef(loader, 0);             // 同时设定参考值为0,这样停止的速度最快
            }
            break;
        default:
            while (1)
                ; // 未知模式,停止运行,检查指针越界,题
    }
#endif
    // 确定是否开启摩擦轮,后续可能修改为键鼠模式下始终开启摩擦轮(上场时建议一直开启)
    if(shoot_cmd_recv.bullet_speed!=last_bullet_speed)
    Get_New_friction_speed(shoot_cmd_recv.bullet_speed);
    last_bullet_speed=shoot_cmd_recv.bullet_speed;
    if (shoot_cmd_recv.friction_mode == FRICTION_ON) {
        DJIMotorSetRef(friction_l, friction_speed);
        DJIMotorSetRef(friction_r, friction_speed);
    } else // 关闭摩擦轮
    {
        DJIMotorSetRef(friction_l, 0);
        DJIMotorSetRef(friction_r, 0);
    }

    // 开关弹舱盖
    if (shoot_cmd_recv.lid_mode == LID_CLOSE) {
        //...
    } else if (shoot_cmd_recv.lid_mode == LID_OPEN) {
        //...
    }

// 反馈数据,目前暂时没有要设定的反馈数据,后续可能增加应用离线监测以及卡弹反馈
#ifdef ONE_BROAD
    PubPushMessage(shoot_pub, (void *)&shoot_feedback_data);
#endif // DEBUG

#ifdef SAMPLING
    if (shoot_cmd_recv.shoot_mode == SHOOT_ON) {
        sampling_result = 5000 * sin_signal_generate(1, 40, 20, flag_complete, flag_frequnency_now);
        DJIMotorOuterLoop(loader, CURRENT_LOOP);
        DJIMotorSetRef(loader, sampling_result);
        store_flag_complete       = *flag_complete;
        store_flag_frequnency_now = *flag_frequnency_now;
        if (store_flag_complete == 1) {
            DJIMotorSetRef(loader, 0);
        }
    }
#endif
}