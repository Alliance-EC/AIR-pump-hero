#include "referee_init.h"
#include "referee_UI.h"
#include "referee_protocol.h"
#include "message_center.h"
#include "rm_referee.h"
#include "UI_def.h"
#include "UI.h"
#include <string.h>
#include "super_cap.h"
#include "general_def.h"
referee_info_t *referee_data_for_ui;

static char char_pitch[50];
static char char_friction_mode[50];
static char char_rotate_mode[50];
static Graph_Data_t Line_fuzhu_center_1, Line_fuzhu_center_2;
static Graph_Data_t Line_fuzhu_5m, Line_fuzhu_10m, Line_fuzhu_8m;
// 辅助线图形变量
// static Graph_Data_t line_fuzhu_nine;
static Graph_Data_t Dirction_Chassis1, Dirction_Chassis2, Dirction_Chassis3, Dirction_Chassis4, Dirction_forward;
static Graph_Data_t Pitch_chassis, Pitch_gimbal, Pitch_dirction;
static String_Data_t CAP_1;
static String_Data_t Image_mode;
static String_Data_t friction_mode;
static String_Data_t rotate_mode;
static String_Data_t HP_remain_worry;
static String_Data_t Load_moad;
static UIdate_for_change UI_last;
static Graph_Data_t Shoot_mode_line1;
static Graph_Data_t Shoot_mode_line2;
static Graph_Data_t Shoot_mode_circle;
static Graph_Data_t Pitch_angle;
static Graph_Data_t Outpost_Hp;
static UIdate_for_change *UI_Now;
uint8_t outpost_zero = 1;
uint8_t check_to_change_UI(UIdate_for_change *UI_now) // 检测UI改变
{
    UI_Now = UI_now;
    return 1;
}

void get_referee_data(referee_info_t *referee_data)
{
    referee_data_for_ui                               = referee_data;
    referee_data_for_ui->referee_id.Robot_Color       = referee_data->GameRobotState.robot_id > 7 ? Robot_Blue : Robot_Red;
    referee_data_for_ui->referee_id.Robot_ID          = referee_data->GameRobotState.robot_id;
    referee_data_for_ui->referee_id.Cilent_ID         = 0x0100 + referee_data->referee_id.Robot_ID; // 计算客户端ID
    referee_data_for_ui->referee_id.Receiver_Robot_ID = 0;
}

static Graph_Data_t CAP_power;
static uint8_t init_flag = 1;
extern SuperCapInstance *cap;
void MyUIInit(void)
{
    UIDelete(&referee_data_for_ui->referee_id, UI_Data_Del_ALL, 0);
    memset(char_friction_mode, '\0', sizeof(friction_mode));
    memset(char_rotate_mode, '\0', sizeof(rotate_mode));
    UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_forward);
    // UILineDraw(&auxiliary_line_fourteen, "114", Graphic_Operate_ADD, 5, Graphic_Color_White, 2, SCREEN_LENGTH / 2 - 30, SCREEN_WIDTH / 2 - 150, SCREEN_LENGTH / 2 - 30, SCREEN_WIDTH / 2);
    // UILineDraw(&Dirction_Gimbal, "GIM", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 2, Center_Of_Dirction_X, Center_Of_Dirction_Y, Center_Of_Dirction_X, Center_Of_Dirction_Y + 100);
    // UIGraphRefresh(&referee_data_for_ui->referee_id, 1, auxiliary_line_fourteen);
    // pitch当前角度，待添加数据
    // sprintf(pitch_data.show_Data, "PITCH");
    // UICharDraw(&pitch_data, "001", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, 80, 880, char_pitch);
    // UICharRefresh(&referee_data_for_ui->referee_id, pitch_data);
    sprintf(CAP_1.show_Data, "CAP:");
    UICharDraw(&CAP_1, "CA1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 - 300, char_pitch);
    UICharRefresh(&referee_data_for_ui->referee_id, CAP_1);
    if (UI_last.shoot_mode == 1) {
        UILineDraw(&Line_fuzhu_center_1, "FZ1", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, center_tigger_X - 50, center_tigger_Y, center_tigger_X + 50, center_tigger_Y);
        UILineDraw(&Line_fuzhu_center_2, "FZ2", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X, center_tigger_Y + 100, center_tigger_X, center_tigger_Y - 200);
        UILineDraw(&Line_fuzhu_5m, "FZ3", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X - 70, center_tigger_Y - 20, center_tigger_X + 70, center_tigger_Y - 20);
        UILineDraw(&Line_fuzhu_10m, "FZ4", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X - 90, center_tigger_Y - 125, center_tigger_X + 90, center_tigger_Y - 125);
        UILineDraw(&Line_fuzhu_8m, "FZ5", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X - 80, center_tigger_Y - 70, center_tigger_X + 80, center_tigger_Y - 70);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_8m);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_10m);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_center_2);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_center_1);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_5m);
        // 画辅助线
    } else {
        UILineDraw(&Shoot_mode_line1, "SM1", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 2, SCREEN_LENGTH / 2 - 60, SCREEN_WIDTH / 2 - 60, SCREEN_LENGTH / 2 + 60, SCREEN_WIDTH / 2 + 60);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line1);
    }
    if (UI_last.chassis_mode == CHASSIS_FOLLOW_GIMBAL_YAW) {
        sprintf(rotate_mode.show_Data, "FOLLOW");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    } else if (UI_last.chassis_mode == CHASSIS_NO_FOLLOW) {
        sprintf(rotate_mode.show_Data, "FREE  ");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    } else {
        sprintf(rotate_mode.show_Data, "ROTATE");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    }
    if (UI_last.remain_HP <= 100) {
        sprintf(HP_remain_worry.show_Data, "RUN RUN RUN");
        UICharDraw(&HP_remain_worry, "004", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 4, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
    } else {
        sprintf(HP_remain_worry.show_Data, "           ");
        UICharDraw(&HP_remain_worry, "004", Graphic_Operate_DEL, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 100, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
    }
    if (referee_data_for_ui->referee_id.Robot_Color == Robot_Red) {
        // sprintf(Robot1.show_Data, "B1");
        // sprintf(Robot2.show_Data, "B2");
        // sprintf(Robot3.show_Data, "B3");
        // sprintf(Robot4.show_Data, "B4");
        // sprintf(Robot5.show_Data, "B5");
        // sprintf(Robotsentry.show_Data, "B7");
        // UICharDraw(&Robot1, "RB1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 400, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot1);
        // UICharDraw(&Robot2, "RB2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 440, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot2);
        // UICharDraw(&Robot3, "RB3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 480, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot3);
        // UICharDraw(&Robot4, "RB4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 520, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot4);
        // UICharDraw(&Robot5, "RB5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 560, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot5);
        // UICharDraw(&Robotsentry, "RB6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 600, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robotsentry);
    } else {
        // sprintf(Robot1.show_Data, "R1");
        // sprintf(Robot2.show_Data, "R2 ");
        // sprintf(Robot3.show_Data, "R3 ");
        // sprintf(Robot4.show_Data, "R4 ");
        // sprintf(Robot5.show_Data, "R5 ");
        // sprintf(Robotsentry.show_Data, "R7");
        // UICharDraw(&Robot1, "RB1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 400, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot1);
        // UICharDraw(&Robot2, "RB2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 440, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot2);
        // UICharDraw(&Robot3, "RB3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 480, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot3);
        // UICharDraw(&Robot4, "RB4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 520, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot4);
        // UICharDraw(&Robot5, "RB5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 560, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robot5);
        // UICharDraw(&Robotsentry, "RB6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 400, SCREEN_WIDTH - 600, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, Robotsentry);
    }
    UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
                 Center_Of_Dirction_X,
                 Center_Of_Dirction_Y, 30);
    UILineDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
               Center_Of_Dirction_X,
               Center_Of_Dirction_Y, Center_Of_Dirction_X, Center_Of_Dirction_Y + 30);
    UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_Chassis1);
    UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_Chassis2);
    init_flag = 1;
    if (UI_Now->vision_servo == Follow_shoot) {
        sprintf(Image_mode.show_Data, "Normal");
        UICharDraw(&Image_mode, "VSM", Graphic_Operate_ADD, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
    } else if (UI_Now->vision_servo == snipe) {
        sprintf(Image_mode.show_Data, "base");
        UICharDraw(&Image_mode, "VSM", Graphic_Operate_ADD, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
    } else if (UI_Now->vision_servo == sentry) {
        sprintf(Image_mode.show_Data, "sentry");
        UICharDraw(&Image_mode, "VSM", Graphic_Operate_ADD, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
    }
    UIIntDraw(&Outpost_Hp, "OTP", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2, 0);
    UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Outpost_Hp);
}

void MyUIRefresh(void)
{
    if (UI_last.outpost_HP != UI_Now->outpost_HP && outpost_zero) {
        UIIntDraw(&Outpost_Hp, "OTP", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2, UI_Now->outpost_HP);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Outpost_Hp);
    }
    if (UI_Now->outpost_HP == 0) {
        UIIntDraw(&Outpost_Hp, "OTP", Graphic_Operate_DEL, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, 0);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Outpost_Hp);
        outpost_zero = 0;
    }
    if (UI_last.vision_servo != UI_Now->vision_servo) {
        if (UI_Now->vision_servo == Follow_shoot) {
            sprintf(Image_mode.show_Data, "normal");
            UILineDraw(&Line_fuzhu_center_1, "FZ1", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, center_tigger_X - 50, center_tigger_Y, center_tigger_X + 50, center_tigger_Y);
            UILineDraw(&Line_fuzhu_center_2, "FZ2", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X, center_tigger_Y + 100, center_tigger_X, center_tigger_Y - 200);
            UILineDraw(&Line_fuzhu_5m, "FZ3", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X - 70, center_tigger_Y - 20, center_tigger_X + 70, center_tigger_Y - 20);
            UILineDraw(&Line_fuzhu_10m, "FZ4", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X - 90, center_tigger_Y - 125, center_tigger_X + 90, center_tigger_Y - 125);
            UILineDraw(&Line_fuzhu_8m, "FZ5", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X - 80, center_tigger_Y - 70, center_tigger_X + 80, center_tigger_Y - 70);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_8m);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_10m);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_center_2);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_center_1);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_5m);
            UICharDraw(&Image_mode, "VSM", Graphic_Operate_DEL, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
            UICharDraw(&Image_mode, "VSM", Graphic_Operate_ADD, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
        } else {
            UILineDraw(&Line_fuzhu_center_1, "FZ1", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 2, Center_Of_Dirction_X - 100, Center_Of_Dirction_Y, Center_Of_Dirction_X + 100, Center_Of_Dirction_Y);
            UILineDraw(&Line_fuzhu_center_2, "FZ2", Graphic_Operate_DEL, 6, Graphic_Color_Purplish_red, 2, Center_Of_Dirction_X, Center_Of_Dirction_Y + 100, Center_Of_Dirction_X, Center_Of_Dirction_Y - 100);
            UILineDraw(&Line_fuzhu_5m, "FZ3", Graphic_Operate_DEL, 6, Graphic_Color_Yellow, 2, Center_Of_Dirction_X - 70, Center_Of_Dirction_Y - 20, Center_Of_Dirction_X + 50, Center_Of_Dirction_Y - 20);
            UILineDraw(&Line_fuzhu_10m, "FZ4", Graphic_Operate_DEL, 6, Graphic_Color_Yellow, 1, center_tigger_X - 90, center_tigger_Y - 40, center_tigger_X + 90, center_tigger_Y - 40);
            UILineDraw(&Line_fuzhu_8m, "FZ5", Graphic_Operate_DEL, 6, Graphic_Color_Yellow, 1, center_tigger_X - 80, center_tigger_Y - 30, center_tigger_X + 80, center_tigger_Y - 30);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_8m);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_10m);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_5m);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_center_2);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_center_1);
            if (UI_Now->vision_servo == snipe) {
                UICharDraw(&Image_mode, "VSM", Graphic_Operate_DEL, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 100, char_pitch);
                UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
                sprintf(Image_mode.show_Data, "Base");
                UICharDraw(&Image_mode, "VSM", Graphic_Operate_ADD, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
                UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
            } else if (UI_Now->vision_servo == sentry) {
                UICharDraw(&Image_mode, "VSM", Graphic_Operate_DEL, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 100, char_pitch);
                UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
                sprintf(Image_mode.show_Data, "Outpost");
                UICharDraw(&Image_mode, "VSM", Graphic_Operate_ADD, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
                UICharRefresh(&referee_data_for_ui->referee_id, Image_mode);
            }
        }
    }
    if (UI_last.Angle != UI_Now->Angle) {
        UILineDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 2,
                   Center_Of_Dirction_X,
                   Center_Of_Dirction_Y, Center_Of_Dirction_X + 60 * arm_cos_f32((-UI_Now->Angle + 90) * DEGREE_2_RAD), Center_Of_Dirction_Y + 40 * arm_sin_f32(((-UI_Now->Angle + 90) * DEGREE_2_RAD)));
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_Chassis2);
    }
    if (UI_last.shoot_mode != UI_Now->shoot_mode) {
        if (UI_Now->shoot_mode == 1) {
            UILineDraw(&Shoot_mode_line1, "SM1", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 2, 0, 0, 0, 0);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line1);
        } else {
            UILineDraw(&Shoot_mode_line1, "SM1", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 2, SCREEN_LENGTH / 2 - 60, SCREEN_WIDTH / 2 - 60, SCREEN_LENGTH / 2 + 60, SCREEN_WIDTH / 2 + 60);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line1);
        }
    }
    if (UI_last.chassis_mode != UI_Now->chassis_mode) {
        if (UI_Now->chassis_mode == CHASSIS_FOLLOW_GIMBAL_YAW) {
            sprintf(rotate_mode.show_Data, "FOLLOW");
            UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
        } else if (UI_Now->chassis_mode == CHASSIS_NO_FOLLOW) {
            sprintf(rotate_mode.show_Data, "FREE  ");
            UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
        } else {
            sprintf(rotate_mode.show_Data, "ROTATE");
            UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
        }
    }
    if (UI_last.remain_HP != UI_Now->remain_HP) {
        if (UI_Now->remain_HP <= UI_Now->Max_HP / 2) {
            sprintf(HP_remain_worry.show_Data, "RUN RUN RUN");
            UICharDraw(&HP_remain_worry, "004", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 5, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
        } else {
            sprintf(HP_remain_worry.show_Data, "           ");
            UICharDraw(&HP_remain_worry, "004", Graphic_Operate_DEL, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
        }
    }
      
    UI_last = *UI_Now;
}

void UI_dirction_draw()
{
    if (init_flag == 1) {
        // UILineDraw(&Pitch_dirction, "PD3", Graphic_Operate_ADD, 8, Graphic_Color_Yellow, 2, Pitch_angle_X,
        //            Pitch_angle_Y + 40, Pitch_angle_X + 60 * arm_cos_f32(-UI_last.pitch_data * DEGREE_2_RAD),
        //            Pitch_angle_Y + 40 + 60 * arm_sin_f32(UI_last.pitch_data * DEGREE_2_RAD));
        // UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Pitch_dirction);
        // UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
        //              Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45) * DEGREE_2_RAD),
        //              Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45) * DEGREE_2_RAD), 30);
        // UICircleDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
        //              Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 90) * DEGREE_2_RAD),
        //              Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 90) * DEGREE_2_RAD), 30);
        // UICircleDraw(&Dirction_Chassis3, "CD3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 2,
        //              Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 180) * DEGREE_2_RAD),
        //              Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 180) * DEGREE_2_RAD), 30);
        // UICircleDraw(&Dirction_Chassis4, "CD4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 2,
        //              Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 270) * DEGREE_2_RAD),
        //              Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 270) * DEGREE_2_RAD), 30);
        // UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Dirction_Chassis1, Dirction_Chassis2);
        // UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Dirction_Chassis3, Dirction_Chassis4);

    } else {
        // UILineDraw(&Pitch_dirction, "PD3", Graphic_Operate_CHANGE, 8, Graphic_Color_Yellow, 2, Pitch_angle_X,
        //            Pitch_angle_Y + 40, Pitch_angle_X + 60 * arm_cos_f32(-UI_last.pitch_data * DEGREE_2_RAD),
        //            Pitch_angle_Y + 40 + 60 * arm_sin_f32(UI_last.pitch_data * DEGREE_2_RAD));
        // UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Pitch_dirction);
        // UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 2,
        //              Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45) * DEGREE_2_RAD),
        //              Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45) * DEGREE_2_RAD), 30);
        // UICircleDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 2,
        //              Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 90) * DEGREE_2_RAD),
        //              Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 90) * DEGREE_2_RAD), 30);
        // UICircleDraw(&Dirction_Chassis3, "CD3", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 2,
        //              Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 180) * DEGREE_2_RAD),
        //              Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 180) * DEGREE_2_RAD), 30);
        // UICircleDraw(&Dirction_Chassis4, "CD4", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 2,
        //              Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 270) * DEGREE_2_RAD),
        //              Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 270) * DEGREE_2_RAD), 30);
        // UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Dirction_Chassis1, Dirction_Chassis2);
        // UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Dirction_Chassis3, Dirction_Chassis4);
    }
}
void UI_ALL_Robot_HP()
{
    // if (referee_data_for_ui->referee_id.Robot_Color == Robot_Blue) {
    //     if (init_flag == 1) {

    //         UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 400, UI_last.All_robot_HP.red_1_robot_HP);
    //         UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 440, UI_last.All_robot_HP.red_2_robot_HP);
    //         UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 480, UI_last.All_robot_HP.red_3_robot_HP);
    //         UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 520, UI_last.All_robot_HP.red_4_robot_HP);
    //         UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 560, UI_last.All_robot_HP.red_5_robot_HP);
    //         UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 600, UI_last.All_robot_HP.red_7_robot_HP);
    //         UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
    //         UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
    //     } else {
    //         UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 400, UI_last.All_robot_HP.red_1_robot_HP);
    //         UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 440, UI_last.All_robot_HP.red_2_robot_HP);
    //         UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 480, UI_last.All_robot_HP.red_3_robot_HP);
    //         UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 520, UI_last.All_robot_HP.red_4_robot_HP);
    //         UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 560, UI_last.All_robot_HP.red_5_robot_HP);
    //         UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 600, UI_last.All_robot_HP.red_7_robot_HP);
    //         UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
    //         UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
    //     }
    // } else {
    //     if (init_flag == 1) {
    //         UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 400, UI_last.All_robot_HP.blue_1_robot_HP);
    //         UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 440, UI_last.All_robot_HP.blue_2_robot_HP);
    //         UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 480, UI_last.All_robot_HP.blue_3_robot_HP);
    //         UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 520, UI_last.All_robot_HP.blue_4_robot_HP);
    //         UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 560, UI_last.All_robot_HP.blue_5_robot_HP);
    //         UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 600, UI_last.All_robot_HP.blue_7_robot_HP);
    //         UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
    //         UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
    //     } else {
    //         UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 400, UI_last.All_robot_HP.blue_1_robot_HP);
    //         UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 440, UI_last.All_robot_HP.blue_2_robot_HP);
    //         UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 480, UI_last.All_robot_HP.blue_3_robot_HP);
    //         UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 520, UI_last.All_robot_HP.blue_4_robot_HP);
    //         UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 560, UI_last.All_robot_HP.blue_5_robot_HP);
    //         UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 600, UI_last.All_robot_HP.blue_7_robot_HP);
    //         UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
    //         UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
    //     }
    // }
}
void UIfresh_Always()
{
    // UI_dirction_draw();
    // UI_ALL_Robot_HP();
    if (init_flag == 1) {
        UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 0, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, UI_last.CapVot * 1000);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, CAP_power);
        UIFloatDraw(&Pitch_angle, "pid", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 0, 2, Pitch_angle_X + 200, Pitch_angle_Y, UI_last.pitch_data * 1000);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Pitch_angle);
        init_flag = 0;
    } else {
        UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 0, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, UI_last.CapVot * 1000);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, CAP_power);
        UIFloatDraw(&Pitch_angle, "pid", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 0, 2, Pitch_angle_X + 200, Pitch_angle_Y, UI_last.pitch_data * 1000);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Pitch_angle);
    }
}