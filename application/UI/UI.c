#include "referee_init.h"
#include "referee_UI.h"
#include "referee_protocol.h"
#include "message_center.h"
#include "rm_referee.h"
#include "UI_def.h"
#include "UI.h"
#include <string.h>
#include "super_cap.h"
static Subscriber_t *referee_sub;

referee_info_t *referee_data_for_ui;

static char char_pitch[50];
static char char_friction_mode[50];
static char char_rotate_mode[50];

// 辅助线图形变量
static Graph_Data_t auxiliary_line_one;
static Graph_Data_t auxiliary_line_two;
static Graph_Data_t auxiliary_line_three;
static Graph_Data_t auxiliary_line_four;
// static Graph_Data_t line_fuzhu_five;
static Graph_Data_t auxiliary_line_six;
static Graph_Data_t auxiliary_line_seven;
static Graph_Data_t auxiliary_line_eight;
// static Graph_Data_t line_fuzhu_nine;
static Graph_Data_t auxiliary_line_ten;
static Graph_Data_t auxiliary_line_eleven;
static Graph_Data_t auxiliary_line_twelve;
static Graph_Data_t auxiliary_line_thirteen;
static Graph_Data_t auxiliary_line_fourteen;
static String_Data_t CAP_1;
static String_Data_t friction_mode;
static String_Data_t rotate_mode;
static String_Data_t HP_remain_worry;
static String_Data_t pitch_data;
static String_Data_t Load_moad;
static UIdate_for_change UI_last;
static uint8_t UIchange_flag = 0;
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

    memset(char_pitch, '\0', sizeof(pitch_data));
    memset(char_friction_mode, '\0', sizeof(friction_mode));
    memset(char_rotate_mode, '\0', sizeof(rotate_mode));

    // 辅助线1
    // UILineDraw(&auxiliary_line_one, "101", Graphic_Operate_ADD, 7, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8 - 90, SCREEN_WIDTH / 2 - 280, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 280);
    // UILineDraw(&auxiliary_line_two, "102", Graphic_Operate_ADD, 7, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8 - 90, SCREEN_WIDTH / 2 - 260, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 260);
    // UILineDraw(&auxiliary_line_three, "101", Graphic_Operate_ADD, 7, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8 - 90, SCREEN_WIDTH / 2 - 240, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 240);
    // UILineDraw(&auxiliary_line_four, "104", Graphic_Operate_ADD, 7, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8 - 90, SCREEN_WIDTH / 2 - 220, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 220);
    //  Draw_Line(&line_fuzhu_five,"095", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 2, SCRE-+]EN_LENGTH / 2 + 8, SCREEN_WIDTH / 2 - 200, SCREEN_LENGTH / 2 + 11.5, SCREEN_WIDTH / 2 - 200);
    // UILineDraw(&auxiliary_line_six, "106", Graphic_Operate_ADD, 7, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8 - 90, SCREEN_WIDTH / 2 - 180, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 180);
    // UILineDraw(&auxiliary_line_seven, "107", Graphic_Operate_ADD, 7, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8 - 90, SCREEN_WIDTH / 2 - 160, SCREEN_LENGTH / 2 + 11.5 + 90 , SCREEN_WIDTH / 2 - 160);
    // UILineDraw(&auxiliary_line_eight, "108", Graphic_Operate_ADD, 7, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8 - 90, SCREEN_WIDTH / 2 - 140, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 140);
    // UIGraphRefresh(&referee_data_for_ui->referee_id, 7, auxiliary_line_one, auxiliary_line_two, auxiliary_line_three, auxiliary_line_four, auxiliary_line_six, auxiliary_line_seven, auxiliary_line_eight);

    // 辅助线2
    //  Draw_Line(&line_fuzhu_nine,"091", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 2, SCREEN_LENGTH / 2 + 8, SCREEN_WIDTH / 2 - 120, SCREEN_LENGTH / 2 + 11.5, SCREEN_WIDTH / 2 - 120);
    // UILineDraw(&auxiliary_line_ten, "110", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8- 90, SCREEN_WIDTH / 2 - 100, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 100);
    // UILineDraw(&auxiliary_line_eleven, "111", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8- 90, SCREEN_WIDTH / 2 - 80, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 80);
    // UILineDraw(&auxiliary_line_twelve, "112", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, SCREEN_LENGTH / 2 + 8- 90, SCREEN_WIDTH / 2 - 60, SCREEN_LENGTH / 2 + 11.5 + 90, SCREEN_WIDTH / 2 - 60);
    // UILineDraw(&auxiliary_line_thirteen, "111", Graphic_Operate_ADD, 5, Graphic_Color_White, 1, SCREEN_LENGTH / 2 + 50, SCREEN_WIDTH / 2 , SCREEN_LENGTH / 2 + 50, SCREEN_WIDTH / 2 );
    UILineDraw(&auxiliary_line_fourteen, "114", Graphic_Operate_ADD, 5, Graphic_Color_White, 2, SCREEN_LENGTH / 2 - 30, SCREEN_WIDTH / 2 - 150, SCREEN_LENGTH / 2 - 30, SCREEN_WIDTH / 2);
    UIGraphRefresh(&referee_data_for_ui->referee_id, 5, auxiliary_line_ten, auxiliary_line_eleven, auxiliary_line_twelve, auxiliary_line_thirteen, auxiliary_line_fourteen);
    // pitch当前角度，待添加数据
    // sprintf(pitch_data.show_Data, "PITCH");
    // UICharDraw(&pitch_data, "001", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, 80, 880, char_pitch);
    // UICharRefresh(&referee_data_for_ui->referee_id, pitch_data);
    sprintf(CAP_1.show_Data, "CAP:");
    UICharDraw(&CAP_1, "CA1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 - 300, char_pitch);
    UICharRefresh(&referee_data_for_ui->referee_id, CAP_1);
    if (UI_last.fir_mode == 1) {
        sprintf(friction_mode.show_Data, "BIU ON");
        UICharDraw(&friction_mode, "002", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, 80, 680, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
    } else {
        sprintf(friction_mode.show_Data, "BIU OFF");
        UICharDraw(&friction_mode, "002", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, 80, 680, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
    }
    if (UI_last.rot_mode == 1) {
        sprintf(rotate_mode.show_Data, "ROT ON");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, 80, 780, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    } else {
        sprintf(rotate_mode.show_Data, "ROT OFF");
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
    init_flag=1;
}

uint8_t check_to_change_UI(UIdate_for_change *UI_now) // 检测UI改变
{
    UIchange_flag  = 0;
    UI_last.Max_HP = UI_now->Max_HP;
    if (UI_now->fir_mode != UI_last.fir_mode) {
        UI_last.fir_mode = UI_now->fir_mode;
        UIchange_flag    = 1;
    }
    if (UI_now->pitch_data != UI_last.pitch_data) {
        UI_last.pitch_data = UI_now->pitch_data;
        UIchange_flag      = 1;
    }
    if (UI_now->rot_mode != UI_last.rot_mode) {
        UI_last.rot_mode = UI_now->rot_mode;
        UIchange_flag    = 1;
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
        sprintf(friction_mode.show_Data, "BIU ON");
        UICharDraw(&friction_mode, "002", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, 80, 680, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
    } else {
        sprintf(friction_mode.show_Data, "BIU OFF");
        UICharDraw(&friction_mode, "002", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, 80, 680, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
    }
    if (UI_last.rot_mode == 1) {
        sprintf(rotate_mode.show_Data, "ROT ON");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, 80, 780, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    } else {
        sprintf(rotate_mode.show_Data, "ROT OFF");
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

void UIfresh_num()
{
    if (init_flag == 1) {
        UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_ADD, 9, Graphic_Color_Yellow,
                    30, 1, 1, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, cap->cap_msg_s.CapVot*1000);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, CAP_power);
    } else {
        UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow,
                    30, 1, 1, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, cap->cap_msg_s.CapVot*1000);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, CAP_power);
    }
    if(1)
    {}
}