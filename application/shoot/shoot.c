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
static DJIMotorInstance *friction_fl, *friction_fr, *friction_bl, *friction_br, *loader; // 拨盘电机
// static servo_instance *lid; 需要增加弹舱盖

static Publisher_t *shoot_pub;
Shoot_Ctrl_Cmd_s shoot_cmd_recv; // 来自cmd的发射控制信息
static Subscriber_t *shoot_sub;
static float loader_angle;
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
    friction_config.can_init_config.tx_id                             = 7;
    friction_fl                                                       = DJIMotorInit(&friction_config); // 前左
    friction_config.can_init_config.tx_id                             = 6;
    friction_bl                                                       = DJIMotorInit(&friction_config); // 后左
    friction_config.can_init_config.tx_id                             = 8;
    friction_config.controller_setting_init_config.motor_reverse_flag = MOTOR_DIRECTION_NORMAL;
    friction_fr                                                       = DJIMotorInit(&friction_config); // 前右

    friction_config.can_init_config.tx_id = 5;
    friction_br                           = DJIMotorInit(&friction_config); // 后右

    // 拨盘电机
    Motor_Init_Config_s loader_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id      = 1,
        },
        .controller_param_init_config = {
            .angle_PID = {
                // 如果启用位置环来控制发弹,需要较大的I值保证输出力矩的线性度否则出现接近拨出的力矩大幅下降
                .Kp     = 20, // 10
                .Ki     = 3,
                .Kd     = 0,
                .MaxOut = 8000,
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
            .other_angle_feedback_ptr = &(loader_angle),
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
static int friction_speed = Inital_Friction_Speed;
static int tick_num1      = 0;
void block_shook_check(float Now_verb_Of_load) // 堵转检测函数
{
    if (fabs(Last_verb_Of_load) <= (loader)->motor_controller.pid_ref / 20 && (loader)->motor_controller.pid_ref != 0) {
        tick_num1++;
        if (tick_num1 >= 10) {
            tick_num1  = 0;
            block_flag = 1;
        }
    } else {
        tick_num1  = 0;
        block_flag = 0;
    }
    Last_verb_Of_load = Now_verb_Of_load;
}
static int tick_block_time = 0;
static uint8_t One_shoot_running;
/* 机器人发射机构控制核心任务 */
void ShootTask()
{
    loader_angle = loader->measure.total_angle;
#ifdef ONE_BROAD
    SubGetMessage(shoot_sub, &shoot_cmd_recv);
#endif // ONE_BORAD
#ifdef GIMBAL_BROAD
// 在robot.c可以获得值
#endif // DEBUG
    if (shoot_cmd_recv.Shoot_power) {
        // 从cmd获取控制数据

        // block_shook_check(loader->measure.speed_aps);
        if (block_flag) {
            DJIMotorSetRef(loader, -1000);
            tick_block_time++;
            if (tick_block_time >= 100000)
                block_flag = 0;
        } else
            tick_block_time = 0;
        // 对shoot mode等于SHOOT_STOP的情况特殊处理,直接停止所有电机(紧急停止)
        if (shoot_cmd_recv.shoot_mode == SHOOT_OFF) {
            DJIMotorStop(friction_bl);
            DJIMotorStop(friction_br);
            DJIMotorStop(friction_fl);
            DJIMotorStop(friction_fr);
            DJIMotorStop(loader);
            DJIMotorSetRef(loader, -loader->measure.total_angle);
        } else // 恢复运行
        {
            DJIMotorEnable(friction_bl);
            DJIMotorEnable(friction_br);
            DJIMotorEnable(friction_fl);
            DJIMotorEnable(friction_fr);
            // if (Shoot_limit_for_oneshootPC6 == 1) {
            //     One_Shoot_flag    = 0;
            //     One_shoot_running = 0;
            // }
            // if (One_shoot_running == 1)
            //     shoot_cmd_recv.load_mode = LOAD_1_BULLET;
            // if (!block_flag) {
            // switch (shoot_cmd_recv.load_mode) {
            //     // 停止拨盘
            //     case LOAD_STOP:
            //         DJIMotorOuterLoop(loader, SPEED_LOOP); // 切换到速度环
            //         DJIMotorSetRef(loader, 0);             // 同时设定参考值为0,这样停止的速度最快
            //         break;
            //     // 单发模式,根据鼠标按下的时间,触发一次之后需要进入不响应输入的状态(否则按下的时间内可能多次进入,导致多次发射)
            //     case LOAD_1_BULLET:
            //         if ((One_Shoot_flag == 1 && shoot_cmd_recv.friction_mode == FRICTION_ON && shoot_cmd_recv.rest_heat == 0) || One_shoot_running) {
            //             DJIMotorOuterLoop(loader, SPEED_LOOP);
            //             DJIMotorEnable(loader);
            //             One_shoot_running = 1;
            //             DJIMotorSetRef(loader, 5000); // 完成1发弹丸发射的时间
            //         } else {
            //             One_shoot_running = 0;
            //             DJIMotorSetRef(loader, 0);
            //         }

            //         break;
            //     case LOAD_MODE: // 装弹模式
            //         if (Shoot_limit_for_oneshootPC6 == 1 && shoot_cmd_recv.friction_mode == FRICTION_ON) {
            //             DJIMotorOuterLoop(loader, SPEED_LOOP);
            //             DJIMotorEnable(loader);
            //             DJIMotorSetRef(loader, 5000);
            //             // 完成1发弹丸发射的时间
            //         } else {
            //             One_Shoot_flag = 1;
            //             DJIMotorOuterLoop(loader, SPEED_LOOP); // 切换到速度环
            //             DJIMotorSetRef(loader, 0);             // 同时设定参考值为0,这样停止的速度最快
            //             DJIMotorStop(loader);
            //         }
            //         break;
            //     case LOAD_BURSTFIRE:
            //         if (shoot_cmd_recv.friction_mode == FRICTION_ON) {
            //             DJIMotorOuterLoop(loader, SPEED_LOOP);
            //             DJIMotorEnable(loader);
            //             DJIMotorSetRef(loader, 8000);
            //         }
            //         break;
            //     default:
            //         break; // 未知模式,停止运行,检查指针越界,题
            // }

            // }
            DJIMotorEnable(loader);
            DJIMotorOuterLoop(loader, ANGLE_LOOP);
            DJIMotorChangeFeed(loader, ANGLE_LOOP, OTHER_FEED);
            if (shoot_cmd_recv.load_mode == LOAD_1_BULLET) {
                if (One_Shoot_flag == 1) {
                    DJIMotorSetRef(loader, -loader->measure.total_angle + ONE_BULLET_DELTA_ANGLE);
                    One_Shoot_flag = 0;
                }

            } else if (shoot_cmd_recv.load_mode == LOAD_MODE) {
                One_Shoot_flag = 1;
            }
            if (shoot_cmd_recv.friction_mode == FRICTION_ON) {
                DJIMotorSetRef(friction_fl, friction_speed + shoot_cmd_recv.friction_speed_adjust * 100);
                DJIMotorSetRef(friction_fr, friction_speed + shoot_cmd_recv.friction_speed_adjust * 100);
            } else // 关闭摩擦轮
            {
                DJIMotorSetRef(friction_bl, -700);
                DJIMotorSetRef(friction_br, -700);
            }
        }
    } else {
        DJIMotorStop(friction_bl);
        DJIMotorStop(friction_br);
        DJIMotorStop(friction_fl);
        DJIMotorStop(friction_fr);
        DJIMotorStop(loader);
    }
#ifdef ONE_BROAD
    PubPushMessage(shoot_pub, (void *)&shoot_feedback_data);
#endif // DEBUG
}