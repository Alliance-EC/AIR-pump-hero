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
static String_Data_t Robot1, Robot2, Robot3, Robot4, Robot5, Robotsentry;
static Graph_Data_t Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP, RobotsentryHP;
// 辅助线图形变量
// static Graph_Data_t line_fuzhu_nine;
static Graph_Data_t Dirction_Chassis, Dirction_Gimbal, Dirction_forward;
static Graph_Data_t auxiliary_line_fourteen;
static String_Data_t CAP_1;
static String_Data_t friction_mode;
static String_Data_t rotate_mode;
static String_Data_t HP_remain_worry;
static String_Data_t pitch_char;
static String_Data_t Load_moad;
static UIdate_for_change UI_last;
static uint8_t UIchange_flag = 0;
static Graph_Data_t Shoot_mode_line1;
static Graph_Data_t Shoot_mode_line2;
static Graph_Data_t Shoot_mode_circle;
static Graph_Data_t Pitch_angle;
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
    UIRectangleDraw(&Dirction_Chassis, "CD1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 2, Center_Of_Dirction_X + Len_Dirction * 0.5,
                    Center_Of_Dirction_Y + Len_Dirction, Center_Of_Dirction_X - Len_Dirction * 0.5,
                    Center_Of_Dirction_Y - Len_Dirction);
    UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_Chassis);
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
    if (UI_last.fir_mode == 1) {
        // 画辅助线
    } else {
        UILineDraw(&Shoot_mode_line1, "SM1", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 2, SCREEN_LENGTH / 2 - 60, SCREEN_WIDTH / 2 - 60, SCREEN_LENGTH / 2 + 60, SCREEN_WIDTH / 2 + 60);
        UILineDraw(&Shoot_mode_line2, "SM2", Graphic_Operate_ADD, 6, Graphic_Color_Purplish_red, 2, SCREEN_LENGTH / 2 + 60, SCREEN_WIDTH / 2 - 60, SCREEN_LENGTH / 2 - 60, SCREEN_WIDTH / 2 + 60);
        UICircleDraw(&Shoot_mode_circle, "SM3", Graphic_Operate_ADD, 4, Graphic_Color_Green, 9, SCREEN_LENGTH, SCREEN_LENGTH, 50);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line1);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line2);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_circle);
    }
    if (UI_last.chassis_mode == CHASSIS_FOLLOW_GIMBAL_YAW) {
        sprintf(rotate_mode.show_Data, "FOLLOW");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, 80, 780, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    } else if (UI_last.chassis_mode == CHASSIS_NO_FOLLOW) {
        sprintf(rotate_mode.show_Data, "FREE  ");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, 80, 780, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    } else {
        sprintf(rotate_mode.show_Data, "ROTATE");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, 80, 780, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    }
    if (UI_last.remain_HP <= 100) {
        sprintf(HP_remain_worry.show_Data, "RUN RUN RUN");
        UICharDraw(&HP_remain_worry, "004", Graphic_Operate_ADD, 9, Graphic_Color_Purplish_red, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
    } else {
        sprintf(HP_remain_worry.show_Data, "           ");
        UICharDraw(&HP_remain_worry, "004", Graphic_Operate_ADD, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
    }
    if (UI_last.load_Mode == 0) {
        sprintf(Load_moad.show_Data, "FIRE     ");
        UICharDraw(&Load_moad, "005", Graphic_Operate_ADD, 9, Graphic_Color_Purplish_red, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Load_moad);
    } else {
        sprintf(Load_moad.show_Data, "ONE SHOOT");
        UICharDraw(&Load_moad, "005", Graphic_Operate_ADD, 9, Graphic_Color_Orange, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Load_moad);
    }
    if (referee_data_for_ui->referee_id.Robot_Color == Robot_Red) {
        sprintf(Robot1.show_Data, "HERO");
        sprintf(Robot2.show_Data, "BLUE2");
        sprintf(Robot3.show_Data, "BLUE3");
        sprintf(Robot4.show_Data, "BLUE4");
        sprintf(Robot5.show_Data, "BLUE5");
        sprintf(Robotsentry.show_Data, "SENTRY");
        UICharDraw(&Robot1, "RB1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 700, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot1);
        UICharDraw(&Robot2, "RB2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot2);
        UICharDraw(&Robot3, "RB3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 300, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot3);
        UICharDraw(&Robot4, "RB4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 400, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot4);
        UICharDraw(&Robot5, "RB5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 500, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot5);
        UICharDraw(&Robotsentry, "RB6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 600, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robotsentry);
    } else {
        sprintf(Robot1.show_Data, "HERO");
        sprintf(Robot2.show_Data, "RED2 ");
        sprintf(Robot3.show_Data, "RED3 ");
        sprintf(Robot4.show_Data, "RED4 ");
        sprintf(Robot5.show_Data, "RED5 ");
        sprintf(Robotsentry.show_Data, "SENTRY");
        UICharDraw(&Robot1, "RB1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 700, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot1);
        UICharDraw(&Robot2, "RB2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot2);
        UICharDraw(&Robot3, "RB3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 300, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot3);
        UICharDraw(&Robot4, "RB4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 400, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot4);
        UICharDraw(&Robot5, "RB5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 500, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robot5);
        UICharDraw(&Robotsentry, "RB6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 300, SCREEN_WIDTH - 600, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Robotsentry);
    }
    init_flag = 1;
}

uint8_t check_to_change_UI(UIdate_for_change *UI_now) // 检测UI改变
{
    UIchange_flag      = 0;
    UI_last.Max_HP     = UI_now->Max_HP;
    UI_last.Angle      = UI_now->Angle;
    UI_last.pitch_data = UI_now->pitch_data;
    if (UI_now->fir_mode != UI_last.fir_mode) {
        UI_last.fir_mode = UI_now->fir_mode;
        UIchange_flag    = 1;
    }
    if (UI_now->pitch_data != UI_last.pitch_data) {
        UI_last.pitch_data = UI_now->pitch_data;
        UIchange_flag      = 1;
    }
    if (UI_now->chassis_mode != UI_last.chassis_mode) {
        UI_last.chassis_mode = UI_now->chassis_mode;
        UIchange_flag        = 1;
    }
    if ((UI_now->remain_HP >= UI_last.Max_HP / 2 || UI_last.remain_HP < UI_last.Max_HP / 2) || ((UI_now->remain_HP < UI_last.Max_HP / 2 || UI_last.remain_HP >= UI_last.Max_HP / 2))) {
        UI_last.remain_HP = UI_now->remain_HP;
        UIchange_flag     = 1;
    }
    if ((UI_now->load_Mode != UI_last.load_Mode)) {
        UI_last.load_Mode = UI_now->load_Mode;
        UIchange_flag     = 1;
    }
    if (UIchange_flag == 1) {
        return 1;
    } else
        return 0;
}
void MyUIRefresh(void)
{
    if (UI_last.fir_mode == 1) {
        UILineDraw(&Shoot_mode_line1, "SM1", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 2, SCREEN_LENGTH / 2 - 60, SCREEN_WIDTH / 2 - 60, SCREEN_LENGTH / 2 + 60, SCREEN_WIDTH / 2 + 60);
        UILineDraw(&Shoot_mode_line2, "SM2", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 2, SCREEN_LENGTH / 2 + 60, SCREEN_WIDTH / 2 + 60, SCREEN_LENGTH / 2 - 60, SCREEN_WIDTH / 2 - 60);
        UICircleDraw(&Shoot_mode_circle, "SM3", Graphic_Operate_DEL, 4, Graphic_Color_Green, 2, SCREEN_LENGTH, SCREEN_LENGTH, 40);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line1);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line2);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_circle); // 画辅助线
    } else {
        UILineDraw(&Shoot_mode_line1, "SM1", Graphic_Operate_CHANGE, 5, Graphic_Color_Purplish_red, 2, SCREEN_LENGTH / 2 - 60, SCREEN_WIDTH / 2 - 60, SCREEN_LENGTH / 2 + 60, SCREEN_WIDTH / 2 + 60);
        UILineDraw(&Shoot_mode_line2, "SM2", Graphic_Operate_CHANGE, 6, Graphic_Color_Purplish_red, 2, SCREEN_LENGTH / 2 + 60, SCREEN_WIDTH / 2 - 60, SCREEN_LENGTH / 2 - 60, SCREEN_WIDTH / 2 + 60);
        UICircleDraw(&Shoot_mode_circle, "SM3", Graphic_Operate_CHANGE, 4, Graphic_Color_Green, 2, SCREEN_LENGTH, SCREEN_LENGTH, 40);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line1);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_line2);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Shoot_mode_circle);
    }
    if (UI_last.chassis_mode == CHASSIS_FOLLOW_GIMBAL_YAW) {
        sprintf(rotate_mode.show_Data, "FOLLOW");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, 80, 780, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    } else if (UI_last.chassis_mode == CHASSIS_NO_FOLLOW) {
        sprintf(rotate_mode.show_Data, "FREE  ");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, 80, 780, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    } else {
        sprintf(rotate_mode.show_Data, "ROTATE");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, 80, 780, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    }
    if (UI_last.remain_HP <= UI_last.Max_HP / 2) {
        sprintf(HP_remain_worry.show_Data, "RUN RUN RUN");
        UICharDraw(&HP_remain_worry, "004", Graphic_Operate_CHANGE, 9, Graphic_Color_Purplish_red, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
    } else {
        sprintf(HP_remain_worry.show_Data, "           ");
        UICharDraw(&HP_remain_worry, "004", Graphic_Operate_CHANGE, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
    }
    if (UI_last.load_Mode == 0) {
        sprintf(Load_moad.show_Data, "FIRE     ");
        UICharDraw(&Load_moad, "005", Graphic_Operate_CHANGE, 9, Graphic_Color_Purplish_red, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Load_moad);
    } else {
        sprintf(Load_moad.show_Data, "ONE SHOOT");
        UICharDraw(&Load_moad, "005", Graphic_Operate_CHANGE, 9, Graphic_Color_Orange, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Load_moad);
    }
}

void UI_dirction_draw()
{
    if (init_flag == 1) {
        UILineDraw(&Dirction_forward, "CD2", Graphic_Operate_ADD, 8, Graphic_Color_Green, 2, Center_Of_Dirction_X,
                   Center_Of_Dirction_Y, Center_Of_Dirction_X + 120 * arm_sin_f32(UI_last.Angle * DEGREE_2_RAD),
                   Center_Of_Dirction_Y + 120 * arm_cos_f32(UI_last.Angle * DEGREE_2_RAD));
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_forward);
    } else {
        UILineDraw(&Dirction_forward, "CD2", Graphic_Operate_CHANGE, 8, Graphic_Color_Green, 2, Center_Of_Dirction_X,
                   Center_Of_Dirction_Y, Center_Of_Dirction_X + 120 * sin(UI_last.Angle * DEGREE_2_RAD),
                   Center_Of_Dirction_Y + 120 * arm_cos_f32(UI_last.Angle * DEGREE_2_RAD));
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_forward);
    }
}
void UI_ALL_Robot_HP()
{
    if (referee_data_for_ui->referee_id.Robot_Color == Robot_Blue) {
        if (init_flag == 1) {

            UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 700, UI_last.All_robot_HP.red_1_robot_HP);
            UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 200, UI_last.All_robot_HP.red_2_robot_HP);
            UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 300, UI_last.All_robot_HP.red_3_robot_HP);
            UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 400, UI_last.All_robot_HP.red_4_robot_HP);
            UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 500, UI_last.All_robot_HP.red_5_robot_HP);
            UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 600, UI_last.All_robot_HP.red_7_robot_HP);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
        } else {
            UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 700, UI_last.All_robot_HP.red_1_robot_HP);
            UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 200, UI_last.All_robot_HP.red_2_robot_HP);
            UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 300, UI_last.All_robot_HP.red_3_robot_HP);
            UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 400, UI_last.All_robot_HP.red_4_robot_HP);
            UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 500, UI_last.All_robot_HP.red_5_robot_HP);
            UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 600, UI_last.All_robot_HP.red_7_robot_HP);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
        }
    } else {
        if (init_flag == 1) {
            UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 700, UI_last.All_robot_HP.blue_1_robot_HP);
            UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 200, UI_last.All_robot_HP.blue_2_robot_HP);
            UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 300, UI_last.All_robot_HP.blue_3_robot_HP);
            UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 400, UI_last.All_robot_HP.blue_4_robot_HP);
            UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 500, UI_last.All_robot_HP.blue_5_robot_HP);
            UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 600, UI_last.All_robot_HP.blue_7_robot_HP);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
        } else {
            UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 700, UI_last.All_robot_HP.blue_1_robot_HP);
            UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 200, UI_last.All_robot_HP.blue_2_robot_HP);
            UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 300, UI_last.All_robot_HP.blue_3_robot_HP);
            UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 400, UI_last.All_robot_HP.blue_4_robot_HP);
            UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 500, UI_last.All_robot_HP.blue_5_robot_HP);
            UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH - 100, SCREEN_WIDTH - 600, UI_last.All_robot_HP.blue_7_robot_HP);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
            UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
        }
    }
}
void UIfresh_Always()
{
    UI_dirction_draw();
    UI_ALL_Robot_HP();
    if (init_flag == 1) {
        UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_ADD, 9, Graphic_Color_Yellow,
                    30, 1, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, cap->cap_msg_s.CapVot * 1000);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, CAP_power);
        UIIntDraw(&Pitch_angle, "pid", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 200, UI_last.pitch_data);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Pitch_angle);
        init_flag = 0;
    } else {
        UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow,
                    30, 1, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, cap->cap_msg_s.CapVot * 1000);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, CAP_power);
        UIIntDraw(&Pitch_angle, "pid", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 200, UI_last.pitch_data);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Pitch_angle);
    }
}