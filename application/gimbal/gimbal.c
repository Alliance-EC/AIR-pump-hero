#include "gimbal.h"
#include "motor_def.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "servo_motor.h"
#include "ins_task.h"
#include "message_center.h"
#include "general_def.h"
#include "can_comm.h"
#include "vision.h"
#include "usart.h"
#include "bmi088.h"
#ifdef GIMBAL_BOARD

CANCommInstance *upboard_can_comm;
#endif
static HostInstance *host_instance; // 上位机接口
// 这里的四元数以wxyz的顺序
static uint8_t vision_recv_data[10]; // 从视觉上位机接收的数据-绝对角度，第9个字节作为识别到目标的标志位
static uint8_t vision_send_data[21]; // 给视觉上位机发送的数据-四元数
static ServoInstance *image_module;  // 图传舵机
static ServoInstance *sight_module;  // 望远镜舵机
INS_Instance *gimbal_IMU_data;       // 云台IMU数据
static DJIMotorInstance *yaw_motor, *pitch_motor;
// 转存遥控器数据，避免在数据传输时使用+=，减轻调试负担
float yaw_input;
float pitch_input;
int single_gimbal;
int gimbal_init_flag; // 用于对电机进行初始输入
#ifdef ONE_BOARD
static Publisher_t *gimbal_pub;  // 云台应用消息发布者(云台反馈给cmd)
static Subscriber_t *gimbal_sub; // cmd控制消息订阅者
#endif                           // DEBUG

Gimbal_Upload_Data_s gimbal_feedback_data; // 回传给cmd的云台状态信息
Gimbal_Ctrl_Cmd_s gimbal_cmd_recv;         // 来自cmd的控制信息
float vision_yaw, vision_pitch, vision_flag;
// 仅供云台内部函数使用
static void GimbalInputGet()
{
    MX_USB_DEVICE_Init();
    memcpy(&vision_yaw, vision_recv_data, sizeof(float));
    memcpy(&vision_pitch, vision_recv_data + 4, sizeof(float));
    // 接受发射指令
    memccpy(&vision_flag, vision_recv_data + 8, sizeof(uint8_t), 1);
    if (vision_flag == 0 || gimbal_cmd_recv.vision_mode == VISION_OFF) {
        yaw_input += gimbal_cmd_recv.yaw / 3.0f;
        pitch_input += gimbal_cmd_recv.pitch / 200;
    } else {
        yaw_input   = vision_yaw + gimbal_IMU_data->INS_data.INS_gyro[INS_YAW_ADDRESS_OFFSET];
        pitch_input = vision_pitch + gimbal_IMU_data->output.INS_angle[INS_PITCH_ADDRESS_OFFSET];
    }
    if (pitch_input > PITCH_MAX_ANGLE)
        pitch_input = PITCH_MAX_ANGLE;
    if (pitch_input < PITCH_MIN_ANGLE)
        pitch_input = PITCH_MIN_ANGLE;
}

void HOST_RECV_CALLBACK()
{
    memcpy(vision_recv_data, host_instance->comm_instance, host_instance->RECV_SIZE);
    vision_recv_data[8] = 1;
}
// 供robot.c调用的外部接口

void GimbalInit()
{
    HostInstanceConf host_conf = {
        .callback  = HOST_RECV_CALLBACK,
        .comm_mode = HOST_VCP,
        .RECV_SIZE = 8,
    };
    host_instance = HostInit(&host_conf); // 视觉通信串口
    Servo_Init_Config_s servo_vision_config = {
        .Channel          = TIM_CHANNEL_1,
        .htim             = &htim1,
        .Servo_Angle_Type = Free_Angle_mode,
        .Servo_type       = Servo180,
    };
    Servo_Init_Config_s servo_sight_config = {
        .Channel          = TIM_CHANNEL_2,
        .htim             = &htim1,
        .Servo_Angle_Type = Free_Angle_mode,
        .Servo_type       = Servo180,
    };
    image_module                = ServoInit(&servo_vision_config);
    sight_module                = ServoInit(&servo_sight_config);
    BMI088_Init_Config_s config = {
        .acc_int_config  = {.GPIOx = GPIOC, .GPIO_Pin = GPIO_PIN_4},
        .gyro_int_config = {.GPIOx = GPIOC, .GPIO_Pin = GPIO_PIN_5},
        .heat_pid_config = {
            .Kp            = 0.32f,
            .Ki            = 0.0004f,
            .Kd            = 0,
            .Improve       = PID_IMPROVE_NONE,
            .IntegralLimit = 0.90f,
            .MaxOut        = 0.95f,
        },
        .heat_pwm_config = {
            .htim      = &htim10,
            .channel   = TIM_CHANNEL_1,
            .dutyratio = 0,
            .period    = 5000 - 1,
        },
        .spi_acc_config = {
            .GPIOx      = GPIOA,
            .cs_pin     = GPIO_PIN_4,
            .spi_handle = &hspi1,
        },
        .spi_gyro_config = {
            .GPIOx      = GPIOB,
            .cs_pin     = GPIO_PIN_0,
            .spi_handle = &hspi1,
        },
        .cali_mode = BMI088_LOAD_PRE_CALI_MODE,
        .work_mode = BMI088_BLOCK_PERIODIC_MODE,

    };
    gimbal_IMU_data = INS_Init(BMI088Register(&config)); // IMU先初始化,获取姿态数据指针赋给yaw电机的其他数据来源
    // YAW
    Motor_Init_Config_s yaw_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id      = 1,
        },
        .controller_param_init_config = {
            .angle_PID = {
                .Kp            = 11, // 8
                .Ki            = 0,
                .Kd            = 0.01,
                .DeadBand      = 0,
                .Improve       = PID_Trapezoid_Intergral | PID_Integral_Limit | PID_Derivative_On_Measurement,
                .IntegralLimit = 6,

                .MaxOut = 500,
            },
            .speed_PID = {
                .Kp            = 40000, // 50
                .Ki            = 10,    // 200
                .Kd            = 0,
                .Improve       = PID_Trapezoid_Intergral | PID_Integral_Limit | PID_Derivative_On_Measurement,
                .IntegralLimit = 4000,
                .DeadBand      = 0,
                .MaxOut        = 30000,
            },
            .other_angle_feedback_ptr = &gimbal_IMU_data->output.Yaw_total_angle,
            // 还需要增加角速度额外反馈指针,注意方向,ins_task.md中有c板的bodyframe坐标系说明
            .other_speed_feedback_ptr = &gimbal_IMU_data->INS_data.INS_gyro[INS_YAW_ADDRESS_OFFSET],
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = OTHER_FEED,
            .outer_loop_type       = ANGLE_LOOP,
            .close_loop_type       = ANGLE_LOOP | SPEED_LOOP,
            .motor_reverse_flag    = MOTOR_DIRECTION_REVERSE,
            .feedback_reverse_flag = FEEDBACK_DIRECTION_REVERSE,
        },
        .motor_type = GM6020};
    // PITCH
    Motor_Init_Config_s pitch_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id      = 5,
        },
        .controller_param_init_config = {
            .angle_PID = {
                .Kp            = 10, // 10
                .Ki            = 0.01,
                .Kd            = 0,
                .Improve       = PID_Trapezoid_Intergral | PID_Integral_Limit | PID_Derivative_On_Measurement,
                .IntegralLimit = 1 + 0,
                .MaxOut        = 500,
            },
            .speed_PID = {
                .Kp            = 10000, // 50
                .Ki            = 2,     // 350
                .Kd            = 0,     // 0
                .Improve       = PID_Trapezoid_Intergral | PID_Integral_Limit | PID_Derivative_On_Measurement,
                .IntegralLimit = 5000,
                .DeadBand      = 0,
                .MaxOut        = 25000,
            },
            .other_angle_feedback_ptr = &gimbal_IMU_data->output.INS_angle[INS_PITCH_ADDRESS_OFFSET],
            // 还需要增加角速度额外反馈指针,注意方向,ins_task.md中有c板的bodyframe坐标系说明
            .other_speed_feedback_ptr = (&gimbal_IMU_data->INS_data.INS_gyro[1]),
        },
        .controller_setting_init_config = {
            .angle_feedback_source = OTHER_FEED,
            .speed_feedback_source = OTHER_FEED,
            .outer_loop_type       = ANGLE_LOOP,
            .close_loop_type       = SPEED_LOOP | ANGLE_LOOP,
            .feedback_reverse_flag = FEEDBACK_DIRECTION_REVERSE,
        },
        .motor_type = GM6020,
    };
    // 电机对total_angle闭环,上电时为零,会保持静止,收到遥控器数据再动
    yaw_motor   = DJIMotorInit(&yaw_config);
    pitch_motor = DJIMotorInit(&pitch_config);
#ifdef GIMBAL_BOARD
    CANComm_Init_Config_s comm_conf = {
        .can_config = {
            .can_handle = &hcan2,
            .tx_id      = 0x312,
            .rx_id      = 0x311,
        },
        .recv_data_len = sizeof(Chassis_Upload_Data_s),
        .send_data_len = sizeof(Chassis_Ctrl_Cmd_s),
    };
    upboard_can_comm = CANCommInit(&comm_conf);
#endif // GIMBAL_BOARD
#ifdef ONE_BOARD
    gimbal_pub = PubRegister("gimbal_feed", sizeof(Gimbal_Upload_Data_s));
    gimbal_sub = SubRegister("gimbal_cmd", sizeof(Gimbal_Ctrl_Cmd_s));
#endif // DEBUG
}

/* 机器人云台控制核心任务,后续考虑只保留IMU控制,不再需要电机的反馈 */
void GimbalTask()
{
    // 获取云台控制数据
    // 后续增加未收到数据的处理
#ifdef ONE_BOARD
    SubGetMessage(gimbal_sub, &gimbal_cmd_recv);
#endif // DEBUG
#ifdef GIMBAL_BOARD
    // 从robot中即可获得gimbal_cmd_recv
#endif // DEBUG

    // @todo:现在已不再需要电机反馈,实际上可以始终使用IMU的姿态数据来作为云台的反馈,yaw电机的offset只是用来跟随底盘
    // 根据控制模式进行电机反馈切换和过渡,视觉模式在robot_cmd模块就已经设置好,gimbal只看yaw_ref和pitch_ref

    switch (gimbal_cmd_recv.sight_mode) {
        case SIGHT_ON:
            Servo_Motor_FreeAngle_Set(sight_module, 174);
            break;
        case SIGHT_OFF:
            Servo_Motor_FreeAngle_Set(sight_module, 80);
            break;
    }

    switch (gimbal_cmd_recv.image_mode) {
         case Follow_shoot:
            Servo_Motor_FreeAngle_Set(image_module,122);
            break;
        case snipe:
            Servo_Motor_FreeAngle_Set(image_module, 88 - gimbal_IMU_data->output.INS_angle_deg[0] * 0.8);
            break;
    }

    switch (gimbal_cmd_recv.gimbal_mode) {
        // 停止
        case GIMBAL_ZERO_FORCE:
            DJIMotorStop(yaw_motor);
            DJIMotorStop(pitch_motor);
            break;
        // 使用陀螺仪的反馈,底盘根据yaw电机的offset跟随云台或视觉模式采用
        case GIMBAL_GYRO_MODE: // 后续只保留此模式
            GimbalInputGet();
            DJIMotorEnable(yaw_motor);
            DJIMotorEnable(pitch_motor);
            DJIMotorChangeFeed(yaw_motor, ANGLE_LOOP, OTHER_FEED);
            DJIMotorChangeFeed(yaw_motor, SPEED_LOOP, OTHER_FEED);
            DJIMotorChangeFeed(pitch_motor, ANGLE_LOOP, OTHER_FEED);
            DJIMotorChangeFeed(pitch_motor, SPEED_LOOP, OTHER_FEED);
            DJIMotorSetRef(yaw_motor, yaw_input); // yaw和pitch会在robot_cmd中处理好多圈和单圈
            DJIMotorSetRef(pitch_motor, pitch_input);
            break;
        // 云台自由模式,使用编码器反馈,底盘和云台分离,仅云台旋转,一般用于调整云台姿态(英雄吊射等)/能量机关
        case GIMBAL_FREE_MODE: // 后续删除,或加入云台追地盘的跟随模式(响应速度更快)
            GimbalInputGet();
            DJIMotorEnable(yaw_motor);
            DJIMotorEnable(pitch_motor);
            DJIMotorChangeFeed(yaw_motor, ANGLE_LOOP, OTHER_FEED);
            DJIMotorChangeFeed(yaw_motor, SPEED_LOOP, OTHER_FEED);
            DJIMotorChangeFeed(pitch_motor, ANGLE_LOOP, OTHER_FEED);
            DJIMotorChangeFeed(pitch_motor, SPEED_LOOP, OTHER_FEED);
            DJIMotorSetRef(yaw_motor, yaw_input); // yaw和pitch会在robot_cmd中处理好多圈和单圈
            DJIMotorSetRef(pitch_motor, pitch_input);
            break;
        default:
            break;
    }

    // 在合适的地方添加pitch重力补偿前馈力矩
    // 根据IMU姿态/pitch电机角度反馈计算出当前配重下的重力矩
    // ...

    // 设置反馈数据,主要是imu和yaw的ecd
    // gimbal_feedback_data.gimbal_imu_data              = gimbal_IMU_data;//需要时可以添加
    

    gimbal_feedback_data.yaw_motor_single_round_angle = (uint16_t)yaw_motor->measure.angle_single_round; // 推送消息
    gimbal_feedback_data.Pitch_data                   = gimbal_IMU_data->output.INS_angle_deg[0];
    // 推送消息
#ifdef ONE_BOARD
    PubPushMessage(gimbal_pub, (void *)&gimbal_feedback_data);
#endif // DEBUG
#ifdef GIMBAL_BOARD

#endif // DEBUG
}

#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
// void UItask(void *argument)
// {
//   /* USER CODE BEGIN Startvisiontask */
//   /* Infinite loop */
//   for(;;)
//   {
//     static uint8_t frame_head[] = {0xAF, 0x32, 0x00, 0x10};
//     memcpy(vision_send_data, frame_head, 4);
//     memcpy(vision_send_data + 4, gimbal_IMU_data->INS_data.INS_quat, sizeof(float) * 4);
//     vision_send_data[20] = 0;
//     for (size_t i = 0; i < 20; i++)
//      vision_send_data[20] += vision_send_data[i];
//     HostSend(host_instance, vision_send_data, 21);
//     osDelay(1);
//   }
//   /* USER CODE END Startvisiontask */
// }
#endif // DEBUG