/**
 * @Author: HDC h2019dc@outlook.com
 * @Date: 2023-09-08 16:47:43
 * @LastEditors: HDC h2019dc@outlook.com
 * @LastEditTime: 2023-10-26 21:51:44
 * @FilePath: \2024_Control_New_Framework_Base-dev-all\application\robot.c
 * @Description:
 *
 * Copyright (c) 2023 by Alliance-EC, All Rights Reserved.
 */
#include "bsp_init.h"
#include "robot.h"
#include "robot_def.h"
#include "robot_task.h"
#include "buzzer.h"
#include "can_comm.h"
#define ROBOT_DEF_PARAM_WARNING
// 编译warning,提醒开发者修改机器人参数
#ifndef ROBOT_DEF_PARAM_WARNING
#define ROBOT_DEF_PARAM_WARNING
#pragma message "check if you have configured the parameters in robot_def.h, IF NOT, please refer to the comments AND DO IT, otherwise the robot will have FATAL ERRORS!!!"
#endif // !ROBOT_DEF_PARAM_WARNING

#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
#include "chassis.h"
#include "robot_cmd.h"
#endif

#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
#include "gimbal.h"
#include "shoot.h"

#endif

void RobotInit()
{
    // 关闭中断,防止在初始化过程中发生中断
    // 请不要在初始化过程中使用中断和延时函数！
    // 若必须,则只允许使用DWT_Delay()
    __disable_irq();

    BSPInit();
   // buzzer_one_note(Do_freq, 0.1f);
#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
    RobotCMDInit();
   // buzzer_one_note(Re_freq, 0.1f);
    ChassisInit();
   // buzzer_one_note(So_freq, 0.1f);
#endif
#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)

    GimbalInit();
   // buzzer_one_note(Mi_freq, 0.1f);
    ShootInit();
   // buzzer_one_note(Fa_freq, 0.1f);
#endif

    // 初始化完成,开启中断
    __enable_irq();
}
#ifdef GIMBAL_BOARD

static Chassis_Upload_Data_s Data_From_Chassis;
extern Shoot_Ctrl_Cmd_s shoot_cmd_recv;
extern Gimbal_Ctrl_Cmd_s gimbal_cmd_recv;
extern Chassis_Ctrl_Cmd_s chassis_cmd_recv;
static Upboard_data Data_From_Gimbal;
extern CANCommInstance *upboard_can_comm;
extern Shoot_Upload_Data_s shoot_feedback_data;
extern Gimbal_Upload_Data_s gimbal_feedback_data;
extern Gimbal_Ctrl_Cmd_s gimbal_cmd_recv;
#endif // DEBUG

void RobotTask()
{
#ifdef CHASSIS_BOARD

#endif // DEBUG
#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
    RobotCMDTask();
    ChassisTask();
#endif
#ifdef GIMBAL_BOARD
    Data_From_Chassis = *(Chassis_Upload_Data_s *)CANCommGet(upboard_can_comm);
    gimbal_cmd_recv   = Data_From_Chassis.gimbal_cmd_upload;
    shoot_cmd_recv    = Data_From_Chassis.shoot_cmd_upload;
#endif // DEBUG
#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
    GimbalTask();
    ShootTask();
#endif
#ifdef GIMBAL_BOARD
    Data_From_Gimbal.Gimbal_data = gimbal_feedback_data;
    Data_From_Gimbal.Shoot_data  = shoot_feedback_data;
    CANCommSend(upboard_can_comm, (void *)&Data_From_Gimbal);
#endif // DEBUG
}
