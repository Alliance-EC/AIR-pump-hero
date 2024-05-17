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
static DJIMotorInstance *loader; // 拨盘电机
// static servo_instance *lid; 需要增加弹舱盖

static Publisher_t *shoot_pub;
Shoot_Ctrl_Cmd_s shoot_cmd_recv; // 来自cmd的发射控制信息
static Subscriber_t *shoot_sub;
Shoot_Upload_Data_s shoot_feedback_data; // 来自cmd的发射控制信息
void ShootInit()
{

    // 拨盘电机
    Motor_Init_Config_s loader_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id      = 1,
        },
        .controller_param_init_config = {
            .angle_PID = {
                // 如果启用位置环来控制发弹,需要较大的I值保证输出力矩的线性度否则出现接近拨出的力矩大幅下降
                .Kp     = 40, // 10
                .Ki     = 0.1,
                .Kd     = 0,
                .MaxOut = 10000,
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
static int8_t photogate_state;
/* 机器人发射机构控制核心任务 */
One_shoot_control NOW_MODE = {LOAD, 1};

static int16_t tick_num1 = 0, tick_num2 = 0;
static int8_t One_Shoot_flag, Last_Air_Mode;
static int8_t Loader_flag = 1;
void One_Shoot_Task()
{
    if (NOW_MODE.now_step == LOAD) {
        // 拨弹盘电机控制
        if (Loader_flag) {
            // DJIMotorOuterLoop(loader, ANGLE_LOOP);
            // DJIMotorSetRef(loader, loader->measure.total_angle - ONE_BULLET_DELTA_ANGLE );
            DJIMotorEnable(loader);
            DJIMotorOuterLoop(loader, SPEED_LOOP);
            DJIMotorSetRef(loader, 4000);
            Loader_flag = 0;
        }
        if (photogate_state == 1) {
            tick_num1 = 0;
            tick_num2++;
        }
        if (tick_num2 >= 100) {
            tick_num2   = 0;
            Loader_flag = 1;
        }
        if (photogate_state == 0) {
            tick_num2 = 0;

            DJIMotorStop(loader);
            tick_num1++;
            if (tick_num1 >= 25) { NOW_MODE.now_step = PUSH; }
        }
    }
    if (NOW_MODE.now_step == PUSH) {
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);
        NOW_MODE.now_step = FIRE;
        osDelay(100);
    }
    if (NOW_MODE.now_step == FIRE) {
        if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_RESET) {
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);
            NOW_MODE.now_step = FIRE;
            osDelay(700);
        }
        if (shoot_cmd_recv.air_pump_mode == AIR_PUMP_ON && One_Shoot_flag) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
            osDelay(100);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_RESET);
            osDelay(600);
            NOW_MODE.now_step = LOAD;
            One_Shoot_flag    = 0;
            Loader_flag       = 1;
        }
    }
}
static uint8_t block_flag = 0;
static uint16_t tick_block;
static float Last_verb_Of_load;
void block_shook_check(float Now_verb_Of_load) // 堵转检测函数
{
    if (
        fabs(Now_verb_Of_load) <= (loader)->motor_controller.pid_ref / 400 && (loader)->motor_controller.pid_ref != 0) {
        tick_block++;
        if (tick_block >= 200) {
            block_flag = 1;
        } else {
            block_flag = 0;
        }
    } else {
        tick_block = 0;
        block_flag = 0;
    }
}
void ShootTask()
{
    // PE14 连推弹锤
    // PC6  连快速排气阀
    // PC7  光电门 v
    // 从cmd获取控制数据
#ifdef ONE_BOARD
    SubGetMessage(shoot_sub, &shoot_cmd_recv);
#endif // ONE_BORAD
#ifdef GIMBAL_BROAD
// 在robot.c可以获得值
#endif // DEBUG
    block_shook_check((loader)->measure.speed_aps);
    if (block_flag == 1) {
        DJIMotorEnable(loader);
        DJIMotorOuterLoop(loader, SPEED_LOOP);
        DJIMotorSetRef(loader, 2000);
        DWT_Delay(0.1);
        DJIMotorSetRef(loader, 0);
        block_flag = 0;
    }
    if (Last_Air_Mode != shoot_cmd_recv.air_pump_mode && shoot_cmd_recv.air_pump_mode == AIR_PUMP_ON) { One_Shoot_flag = 1; }
    Last_Air_Mode   = shoot_cmd_recv.air_pump_mode;
    photogate_state = HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_6);
    if (shoot_cmd_recv.shoot_mode == SHOOT_OFF) {
        DJIMotorStop(loader);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_RESET);
    } else {
        One_Shoot_Task();
    }
    shoot_feedback_data.bullet_ready = NOW_MODE.now_step;
#ifdef ONE_BOARD
    PubPushMessage(shoot_pub, (void *)&shoot_feedback_data);
#endif // DEBUG
}