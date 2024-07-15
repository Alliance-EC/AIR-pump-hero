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
// static char char_friction_mode[50];
// static char char_rotate_mode[50];
// static String_Data_t Robot1, Robot2, Robot3, Robot4, Robot5, Robotsentry;
// static Graph_Data_t Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP, RobotsentryHP;
static Graph_Data_t Line_fuzhu_base1, Line_fuzhu_base2;
static Graph_Data_t Line_fuzhu_5m, Line_fuzhu_10m, Line_fuzhu_8m;
static Graph_Data_t Line_fuzhu_outposefirst1, Line_fuzhu_outposefirst2;
// 辅助线图形变量
static Graph_Data_t Dirction_Chassis1, Dirction_Chassis2, Dirction_Chassis3, Dirction_Chassis4;
static Graph_Data_t Shoot_enemy1, Shoot_enemy2;
static Graph_Data_t Bullet_worry1, Bullet_worry2, Bullet_worry3;
static String_Data_t CAP_1;
static String_Data_t friction_mode;
static String_Data_t rotate_mode;
static String_Data_t HP_remain_worry;
static String_Data_t Load_moad;
static UIdate_for_change UI_last;
static Graph_Data_t NUMX;
static Graph_Data_t NUMY;
static Graph_Data_t MAX_POWER;
static Graph_Data_t Pitch_angle;
static Graph_Data_t Friction_speed;
static UIdate_for_change *UI_Now;
int16_t Outpose_X2 = Center_outpose_firstX, Outpose_Y2 = Center_outpose_firstY;
int16_t Outpose_X = Center_outpose_firstX, Outpose_Y = Center_outpose_firstY;
int16_t case_7m_X = Center_base_7m_X, case_7m_Y = Center_base_7m_Y;
int16_t case_20m_X = Center_base_20m_X, case_20m_Y = Center_base_20m_Y;
int UI_flag_second = 0;
#define MAX_UI_INSTANCE 30
static UI_Graph_Instance *UI_INSTANCE[MAX_UI_INSTANCE];
static int8_t CNT_UI;
static int tick_num_bulletUI;
UI_Graph_Instance *UI_LINE_Init(char Name[3], int8_t color, int8_t layer, int8_t width, int16_t startx, int16_t starty, int16_t endx, int16_t endy)
{
    UI_Graph_Instance *UI_instance = (UI_Graph_Instance *)malloc(sizeof(UI_Graph_Instance));
    memset(UI_instance, 0, sizeof(UI_Graph_Instance));

    UI_instance->Color = color;
    UI_instance->Layer = layer;
    strcpy(UI_instance->name, Name);
    UI_instance->coordinate_axis[0] = startx;
    UI_instance->coordinate_axis[1] = starty;
    UI_instance->coordinate_axis[2] = endx;
    UI_instance->coordinate_axis[3] = endy;
    UI_instance->TYPE               = LINE;
    UI_instance->init_flag          = 1;
    UI_INSTANCE[CNT_UI++]           = UI_instance;

    return UI_instance;
}
void UI_Graph_change(UI_Graph_Instance *UI_GRAPH)
{
    switch (UI_GRAPH->TYPE) {
        case LINE:
            if (UI_GRAPH->init_flag == 1) {
                UI_GRAPH->init_flag = 0;
                UILineDraw(&(UI_GRAPH->UI_instantiation), UI_GRAPH->name, Graphic_Operate_ADD, UI_GRAPH->Layer, UI_GRAPH->Color, UI_GRAPH->Width, UI_GRAPH->coordinate_axis[0], UI_GRAPH->coordinate_axis[1], UI_GRAPH->coordinate_axis[2], UI_GRAPH->coordinate_axis[3]);
            } else
                UILineDraw(&(UI_GRAPH->UI_instantiation), UI_GRAPH->name, Graphic_Operate_CHANGE, UI_GRAPH->Layer, UI_GRAPH->Color, UI_GRAPH->Width, UI_GRAPH->coordinate_axis[0], UI_GRAPH->coordinate_axis[1], UI_GRAPH->coordinate_axis[2], UI_GRAPH->coordinate_axis[3]);
            break;
        case Circle:
            break;
        case Retangle:
            break;
        case ARC:
            break;
        case INT_TYPE:
            break;
        case OVAL:
            break;
        case FLOAT_TYPE:
            break;
    }
}
uint8_t Change_UI_Data(UIdate_for_change *UI_now) // 检测UI改变
{
    UI_Now  = UI_now;
    UI_last = *UI_Now;
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
void UI_DEBUG_MODE(uint8_t CNT, int px, int py)
{
    if (CNT == 0) {
        Outpose_X = Center_outpose_firstX - px;
        Outpose_Y = Center_outpose_firstY - py;
        UIIntDraw(&NUMX, "NMX", Graphic_Operate_ADD, 9, Graphic_Color_White, 30, 2, SCREEN_LENGTH / 2 - 600, SCREEN_WIDTH / 2 - 100, Outpose_X);
        UIIntDraw(&NUMY, "NMY", Graphic_Operate_ADD, 9, Graphic_Color_White, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 100, Outpose_Y);
        UILineDraw(&Line_fuzhu_outposefirst1, "FO1", Graphic_Operate_CHANGE, 5, Graphic_Color_Yellow, 2, Outpose_X + 20, Outpose_Y, Outpose_X - 20, Outpose_Y);
        UILineDraw(&Line_fuzhu_outposefirst2, "FO2", Graphic_Operate_CHANGE, 5, Graphic_Color_Yellow, 2, Outpose_X, Outpose_Y + 20, Outpose_X, Outpose_Y - 20

        );
        UIGraphRefresh(&referee_data_for_ui->referee_id, 2, NUMX, NUMY);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Line_fuzhu_outposefirst1, Line_fuzhu_outposefirst2);
    }
    if (CNT == 1) {
        Outpose_X = Center_base_7m_X - px;
        Outpose_Y = Center_base_7m_Y - py;
        UIIntDraw(&NUMX, "NMX", Graphic_Operate_ADD, 9, Graphic_Color_White, 30, 2, SCREEN_LENGTH / 2 - 600, SCREEN_WIDTH / 2 - 100, Center_base_7m_X);
        UIIntDraw(&NUMY, "NMY", Graphic_Operate_ADD, 9, Graphic_Color_White, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 100, Center_base_7m_Y);
        UILineDraw(&Line_fuzhu_base1, "FB1", Graphic_Operate_CHANGE, 5, Graphic_Color_Yellow, 2, Center_base_7m_X + 20, Center_base_7m_Y, Center_base_7m_X - 20, Center_base_7m_Y);
        UILineDraw(&Line_fuzhu_base2, "FB2", Graphic_Operate_CHANGE, 5, Graphic_Color_Yellow, 2, Center_base_7m_X, Center_base_7m_Y + 20, Center_base_7m_X, Center_base_7m_Y - 20);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 2, NUMX, NUMY);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Line_fuzhu_base1, Line_fuzhu_base2);
    }
    if (CNT == 2) {
        Outpose_X = Center_outpose_firstX - px;
        Outpose_Y = Center_outpose_firstY - py;
        UIIntDraw(&NUMX, "NMX", Graphic_Operate_ADD, 9, Graphic_Color_White, 30, 2, SCREEN_LENGTH / 2 - 600, SCREEN_WIDTH / 2 - 100, Outpose_X);
        UIIntDraw(&NUMY, "NMY", Graphic_Operate_ADD, 9, Graphic_Color_White, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 100, Outpose_Y);
        UILineDraw(&Line_fuzhu_outposefirst1, "FO1", Graphic_Operate_CHANGE, 5, Graphic_Color_Yellow, 2, Outpose_X + 10, Outpose_Y, Outpose_X - 10, Outpose_Y);
        UILineDraw(&Line_fuzhu_outposefirst2, "FO2", Graphic_Operate_CHANGE, 5, Graphic_Color_Yellow, 2, Outpose_X, Outpose_Y + 10, Outpose_X, Outpose_Y - 10);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 2, NUMX, NUMY);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Line_fuzhu_outposefirst1, Line_fuzhu_outposefirst2);
    } else
        return;
}
static Graph_Data_t CAP_power;
static uint8_t init_flag = 1;
extern SuperCapInstance *cap;
void MyUIInit(void)
{
    for (int i = 0; i < CNT_UI; i++) {
        UI_Graph_change(UI_INSTANCE[i]);
    }
    if (UI_flag_second == 0) {
        // memset(char_friction_mode, '\0', sizeof(friction_mode));
        // memset(char_rotate_mode, '\0', sizeof(rotate_mode));

        // UILineDraw(&auxiliary_line_fourteen, "114", Graphic_Operate_ADD, 5, Graphic_Color_White, 2, SCREEN_LENGTH / 2 - 30, SCREEN_WIDTH / 2 - 150, SCREEN_LENGTH / 2 - 30, SCREEN_WIDTH / 2);
        // UILineDraw(&Dirction_Gimbal, "GIM", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 2, Center_Of_Dirction_X, Center_Of_Dirction_Y, Center_Of_Dirction_X, Center_Of_Dirction_Y + 100);
        // UIGraphRefresh(&referee_data_for_ui->referee_id, 1, auxiliary_line_fourteen);
        // pitch当前角度，待添加数据
        // sprintf(pitch_data.show_Data, "PITCH");
        // UICharDraw(&pitch_data, "001", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, 80, 880, char_pitch);
        // UICharRefresh(&referee_data_for_ui->referee_id, pitch_data);
        sprintf(CAP_1.show_Data, "CAP:");
        UICharDraw(&CAP_1, "CA1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 - 300, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, CAP_1); // 电容CAP字符初始化
        sprintf(Load_moad.show_Data, "SHOOT_MOD");
        UICharDraw(&Load_moad, "LDM", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 + 100, SCREEN_WIDTH / 2 + 100, char_pitch);
        UICharRefresh(&referee_data_for_ui->referee_id, Load_moad);
        if (UI_last.chassis_mode == CHASSIS_FOLLOW_GIMBAL_YAW) {
            sprintf(rotate_mode.show_Data, "FOLLOW");
            UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
        } else if (UI_last.chassis_mode == CHASSIS_NO_FOLLOW) {
            sprintf(rotate_mode.show_Data, "FREE  ");
            UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
        } else {
            sprintf(rotate_mode.show_Data, "ROTATE");
            UICharDraw(&rotate_mode, "003", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2 + 200, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
        } // 底盘状态字符初始化

        // if (UI_last.remain_HP <= 100) {
        //     sprintf(HP_remain_worry.show_Da     ta, "RUN RUN RUN");
        //     UICharDraw(&HP_remain_worry, "004", Graphic_Operate_ADD, 9, Graphic_Color_Purplish_red, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
        //     UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
        // } else {
        //     sprintf(HP_remain_worry.show_Data, "           ");
        //     UICharDraw(&HP_remain_worry, "004", Graphic_Operate_ADD, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
        //     UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
        // }

        UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
                     Center_Of_Dirction_X + 80 * arm_cos_f32((-UI_Now->Angle + 90) * DEGREE_2_RAD), Center_Of_Dirction_Y + 80 * arm_sin_f32((-UI_Now->Angle + 90) * DEGREE_2_RAD), 8);
        UICircleDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_ADD, 9, Graphic_Color_White, 2,
                     Center_Of_Dirction_X + 80 * arm_cos_f32((-UI_Now->Angle + 180) * DEGREE_2_RAD), Center_Of_Dirction_Y + 80 * arm_sin_f32((-UI_Now->Angle + 180) * DEGREE_2_RAD), 8);
        UICircleDraw(&Dirction_Chassis3, "CD3", Graphic_Operate_ADD, 9, Graphic_Color_White, 2,
                     Center_Of_Dirction_X + 80 * arm_cos_f32((-UI_Now->Angle + 270) * DEGREE_2_RAD), Center_Of_Dirction_Y + 80 * arm_sin_f32((-UI_Now->Angle + 270) * DEGREE_2_RAD), 8);
        UICircleDraw(&Dirction_Chassis4, "CD4", Graphic_Operate_ADD, 9, Graphic_Color_White, 2,
                     Center_Of_Dirction_X + 80 * arm_cos_f32((-UI_Now->Angle + 360) * DEGREE_2_RAD), Center_Of_Dirction_Y + 80 * arm_sin_f32((-UI_Now->Angle + 360) * DEGREE_2_RAD), 8);
        // YAW朝向更新
        //  更改表现形式
        //  UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
        //               Center_Of_Dirction_X,
        //               Center_Of_Dirction_Y, 30);
        //  UILineDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
        //             Center_Of_Dirction_X,
        //             Center_Of_Dirction_Y, Center_Of_Dirction_X, Center_Of_Dirction_Y + 30);
        //  UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_Chassis1);
        //  UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_Chassis2);//云台yaw朝向初始化
        UILineDraw(&Line_fuzhu_outposefirst1, "FO1", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, Center_base_7m_X + 20, Center_outpose_firstY, Center_base_7m_X - 20, Center_outpose_firstY);
        UILineDraw(&Line_fuzhu_outposefirst2, "FO2", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, Center_outpose_firstX, Center_outpose_firstY + 20, Center_outpose_firstX, Center_outpose_firstY - 40);
        UILineDraw(&Line_fuzhu_base1, "FB1", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, Center_base_7m_X + 20, Center_base_7m_Y, Center_base_7m_X - 20, Center_base_7m_Y);
        UILineDraw(&Line_fuzhu_base2, "FB2", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, Center_base_7m_X, Center_base_7m_Y + 80, Center_base_7m_X, Center_base_7m_Y - 200);

        // UILineDraw(&Line_fuzhu_outpost, "FZ6", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, center_tigger_X - 80, 605, center_tigger_X + 80, 605);
        // UILineDraw(&Line_fuzhu_center_1, "FZ1", Graphic_Operate_ADD, 5, Graphic_Color_Yellow, 1, center_tigger_X - 50, center_tigger_Y, center_tigger_X + 50, center_tigger_Y);
        // UILineDraw(&Line_fuzhu_center_2, "FZ2", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X, center_tigger_Y + 200, center_tigger_X, center_tigger_Y - 200);
        // UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Line_fuzhu_outpost);
        // UILineDraw(&Line_fuzhu_5m, "FZ3", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X - 70, center_tigger_Y - 20, center_tigger_X + 70, center_tigger_Y - 20);
        // UILineDraw(&Line_fuzhu_10m, "FZ4", Graphic_Operate_ADD, 6, Graphic_Color_Yellow, 1, center_tigger_X - 90, center_tigger_Y - 125, center_tigger_X + 90, center_tigger_Y - 125);
        // UILineDraw(&Line_fuzhu_8m, "FZ5", Graphic_Operate_ADD, 6, Graphic_Color_White, 1, center_tigger_X - 80, center_tigger_Y - 70, center_tigger_X + 80, center_tigger_Y - 70);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Line_fuzhu_8m, Dirction_Chassis1, Dirction_Chassis2, Dirction_Chassis3, Dirction_Chassis4); // 辅助线整体初始化
        // 辅助线整体初始化
        UILineDraw(&Shoot_enemy1, "SE1", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 1, SCREEN_LENGTH / 2 + 80, SCREEN_WIDTH / 2 + 80, SCREEN_LENGTH / 2 - 80, SCREEN_WIDTH / 2 - 80);
        UILineDraw(&Shoot_enemy2, "SE2", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 1, SCREEN_LENGTH / 2 - 80, SCREEN_WIDTH / 2 + 80, SCREEN_LENGTH / 2 + 80, SCREEN_WIDTH / 2 - 80);
        // 热量超载UI

        UILineDraw(&Bullet_worry1, "BW1", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X - 40, Center_bullet_worry_Y, Center_bullet_worry_X + 40, Center_bullet_worry_Y);
        UILineDraw(&Bullet_worry2, "BW2", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X + 40, Center_bullet_worry_Y, Center_bullet_worry_X + 20, Center_bullet_worry_Y - 20);
        UILineDraw(&Bullet_worry3, "BW3", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X + 40, Center_bullet_worry_Y, Center_bullet_worry_X + 20, Center_bullet_worry_Y + 20);
        // 弹量告急UI

        if (UI_last.fri_mode == FRICTION_ON) {
            sprintf(friction_mode.show_Data, "BIU ON ");
            UICharDraw(&friction_mode, "002", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
        } else {
            sprintf(friction_mode.show_Data, "BIU OFF");
            UICharDraw(&friction_mode, "002", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2, char_pitch);
            UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
        } // 摩擦轮状态初始化
        UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 0, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, UI_last.CapVot * 1000);
        UIFloatDraw(&Pitch_angle, "PTD", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 30, 0, 2, SCREEN_LENGTH / 2 + 100, SCREEN_WIDTH / 2, UI_last.pitch_data * 1000);
        UIIntDraw(&Friction_speed, "FRS", Graphic_Operate_ADD, 9, Graphic_Color_White, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 50, UI_last.Frition_speed);
        UIIntDraw(&MAX_POWER, "MAP", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 30, 2, Max_Power_X, Max_Power_Y, UI_last.Max_power);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 7, Shoot_enemy1, Shoot_enemy2, Bullet_worry1, Bullet_worry2, Bullet_worry3, Line_fuzhu_outposefirst1, Line_fuzhu_outposefirst2);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Friction_speed, Pitch_angle, CAP_power, Line_fuzhu_base1, Line_fuzhu_base2);
        UIGraphRefresh(&referee_data_for_ui->referee_id, 1, MAX_POWER);
        UI_flag_second = 1;
    } else {
        MyUIRefresh();
    }
}

void MyUIRefresh(void)
{
    // if (UI_last.shoot_mode != UI_Now->shoot_mode) {
    //     if (UI_Now->shoot_mode == 1) {
    //         sprintf(friction_mode.show_Data, "BIU ON ");
    //         UICharDraw(&friction_mode, "002", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 , char_pitch);
    //         UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
    //     } else {
    //         sprintf(friction_mode.show_Data, "BIU OFF");
    //         UICharDraw(&friction_mode, "002", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 , char_pitch);
    //         UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
    //     }
    // }

    // if (UI_last.load_Mode != UI_Now->shoot_mode) {
    //     if (UI_last.load_Mode == 0) {
    //         sprintf(Load_moad.show_Data, "FIRE     ");
    //         UICharDraw(&Load_moad, "006", Graphic_Operate_CHANGE, 9, Graphic_Color_Purplish_red, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
    //         UICharRefresh(&referee_data_for_ui->referee_id, Load_moad);
    //     } else {
    //         sprintf(Load_moad.show_Data, "ONE SHOOT");
    //         UICharDraw(&Load_moad, "006", Graphic_Operate_CHANGE, 9, Graphic_Color_Orange, 30, 2, SCREEN_LENGTH / 2 + 400, SCREEN_WIDTH / 2 + 200, char_pitch);
    //         UICharRefresh(&referee_data_for_ui->referee_id, Load_moad);
    //     }
    // }
    // if (UI_last.Angle != UI_Now->Angle) {
    //     UILineDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 2,
    //                Center_Of_Dirction_X,
    //                Center_Of_Dirction_Y, Center_Of_Dirction_X + 60 * arm_cos_f32((-UI_Now->Angle + 90) * DEGREE_2_RAD), Center_Of_Dirction_Y + 40 * arm_sin_f32(((-UI_Now->Angle + 90) * DEGREE_2_RAD)));
    //     UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Dirction_Chassis2);
    // }
    // if (UI_Now->chassis_mode == CHASSIS_FOLLOW_GIMBAL_YAW) {
    //     sprintf(rotate_mode.show_Data, "FOLLOW");
    //     UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
    //     UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    // } else if (UI_Now->chassis_mode == CHASSIS_NO_FOLLOW) {
    //     sprintf(rotate_mode.show_Data, "FREE  ");
    //     UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
    //     UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    // } else {
    //     sprintf(rotate_mode.show_Data, "ROTATE");
    //     UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
    //     UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    // }
    // if (UI_last.remain_HP != UI_Now->remain_HP) {
    //     if (UI_Now->remain_HP <= UI_Now->Max_HP / 2) {
    //         sprintf(HP_remain_worry.show_Data, "RUN RUN RUN");
    //         UICharDraw(&HP_remain_worry, "004", Graphic_Operate_CHANGE, 9, Graphic_Color_Purplish_red, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
    //         UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
    //     } else {
    //         sprintf(HP_remain_worry.show_Data, "           ");
    //         UICharDraw(&HP_remain_worry, "004", Graphic_Operate_CHANGE, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 200, SCREEN_WIDTH / 2 + 200, char_pitch);
    //         UICharRefresh(&referee_data_for_ui->referee_id, HP_remain_worry);
    //     }
    // // }

    // UICharDraw(&friction_mode, "002", Graphic_Operate_DEL, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2, char_pitch);
    // UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_DEL, 9, Graphic_Color_Pink, 2,
    //              Center_Of_Dirction_X + 80 * arm_cos_f32((-UI_Now->Angle + 90) * DEGREE_2_RAD), Center_Of_Dirction_Y + 80 * arm_sin_f32((-UI_Now->Angle + 90) * DEGREE_2_RAD), 15);
    // UICircleDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_DEL, 9, Graphic_Color_White, 2,
    //              Center_Of_Dirction_X + 80 * arm_cos_f32((-UI_Now->Angle + 180) * DEGREE_2_RAD), Center_Of_Dirction_Y + 80 * arm_sin_f32((-UI_Now->Angle + 180) * DEGREE_2_RAD), 15);
    // UICircleDraw(&Dirction_Chassis3, "CD3", Graphic_Operate_DEL, 9, Graphic_Color_White, 2,
    //              Center_Of_Dirction_X + 80 * arm_cos_f32((-UI_Now->Angle + 270) * DEGREE_2_RAD), Center_Of_Dirction_Y + 80 * arm_sin_f32((-UI_Now->Angle + 270) * DEGREE_2_RAD), 15);
    // UICircleDraw(&Dirction_Chassis4, "CD4", Graphic_Operate_DEL, 9, Graphic_Color_White, 2,
    //              Center_Of_Dirction_X + 80 * arm_cos_f32((-UI_Now->Angle + 360) * DEGREE_2_RAD), Center_Of_Dirction_Y + 80 * arm_sin_f32((-UI_Now->Angle + 360) * DEGREE_2_RAD), 15);
    // UICharDraw(&rotate_mode, "003", Graphic_Operate_DEL, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 400, SCREEN_WIDTH / 2 + 200, char_pitch);
    // UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_DEL, 9, Graphic_Color_Yellow, 30, 0, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, UI_Now->CapVot * 1000);
    // UIFloatDraw(&Pitch_angle, "PTD", Graphic_Operate_DEL, 9, Graphic_Color_Yellow, 30, 0, 2, SCREEN_LENGTH / 2 + 100, SCREEN_WIDTH / 2, UI_Now->pitch_data * 1000);
    if (UI_Now->load_Mode == One_shoot_mdoe) {
        sprintf(Load_moad.show_Data, "ONE_SHOOT");
        UICharDraw(&Load_moad, "LDM", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 + 300, SCREEN_WIDTH / 2 + 200, char_pitch);

    } else {
        sprintf(Load_moad.show_Data, "FIRE     ");
        UICharDraw(&Load_moad, "LDM", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 30, 2, SCREEN_LENGTH / 2 + 300, SCREEN_WIDTH / 2 + 200, char_pitch);
    }
    UICharRefresh(&referee_data_for_ui->referee_id, Load_moad);
    if (UI_Now->fri_mode == FRICTION_ON) {
        sprintf(friction_mode.show_Data, "BIU ON ");
        UICharDraw(&friction_mode, "002", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2, char_pitch);
    } else {
        sprintf(friction_mode.show_Data, "BIU OFF");
        UICharDraw(&friction_mode, "002", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2, char_pitch);
    } // 摩擦轮状态更新
    UIIntDraw(&Friction_speed, "FRS", Graphic_Operate_CHANGE, 9, Graphic_Color_White, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2 + 50, UI_last.Frition_speed);
    UIIntDraw(&MAX_POWER, "MAP", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 30, 2, Max_Power_X, Max_Power_Y, UI_last.Max_power);

    UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 1,
                 Center_Of_Dirction_X + 40 * arm_cos_f32((-UI_Now->Angle + 90) * DEGREE_2_RAD), Center_Of_Dirction_Y + 40 * arm_sin_f32((-UI_Now->Angle + 90) * DEGREE_2_RAD), 8);
    UICircleDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_CHANGE, 9, Graphic_Color_White, 1,
                 Center_Of_Dirction_X + 40 * arm_cos_f32((-UI_Now->Angle + 180) * DEGREE_2_RAD), Center_Of_Dirction_Y + 40 * arm_sin_f32((-UI_Now->Angle + 180) * DEGREE_2_RAD), 8);
    UICircleDraw(&Dirction_Chassis3, "CD3", Graphic_Operate_CHANGE, 9, Graphic_Color_White, 1,
                 Center_Of_Dirction_X + 40 * arm_cos_f32((-UI_Now->Angle + 270) * DEGREE_2_RAD), Center_Of_Dirction_Y + 40 * arm_sin_f32((-UI_Now->Angle + 270) * DEGREE_2_RAD), 8);
    UICircleDraw(&Dirction_Chassis4, "CD4", Graphic_Operate_CHANGE, 9, Graphic_Color_White, 1,
                 Center_Of_Dirction_X + 40 * arm_cos_f32((-UI_Now->Angle + 360) * DEGREE_2_RAD), Center_Of_Dirction_Y + 40 * arm_sin_f32((-UI_Now->Angle + 360) * DEGREE_2_RAD), 8);

    if (UI_Now->chassis_mode == CHASSIS_FOLLOW_GIMBAL_YAW) {
        sprintf(rotate_mode.show_Data, "FOLLOW");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2 + 200, char_pitch);
    } else if (UI_Now->chassis_mode == CHASSIS_NO_FOLLOW) {
        sprintf(rotate_mode.show_Data, "FREE  ");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Green, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2 + 200, char_pitch);
    } else {
        sprintf(rotate_mode.show_Data, "ROTATE");
        UICharDraw(&rotate_mode, "003", Graphic_Operate_CHANGE, 9, Graphic_Color_Orange, 30, 2, SCREEN_LENGTH / 2 - 350, SCREEN_WIDTH / 2 + 200, char_pitch);
    } // 底盘状态更新

    switch (UI_Now->Bullet_empty) {
        case 0:
            UILineDraw(&Bullet_worry1, "BW1", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X - 40, Center_bullet_worry_Y, Center_bullet_worry_X + 40, Center_bullet_worry_Y);
            UILineDraw(&Bullet_worry2, "BW2", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X + 40, Center_bullet_worry_Y, Center_bullet_worry_X + 20, Center_bullet_worry_Y - 20);
            UILineDraw(&Bullet_worry3, "BW3", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X + 40, Center_bullet_worry_Y, Center_bullet_worry_X + 20, Center_bullet_worry_Y + 20);
            break;
        case 1:
            tick_num_bulletUI++;
            if (tick_num_bulletUI > 5) {
                UILineDraw(&Bullet_worry1, "BW1", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X - 80, Center_bullet_worry_Y, Center_bullet_worry_X + 40, Center_bullet_worry_Y);
                UILineDraw(&Bullet_worry2, "BW2", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X + 40, Center_bullet_worry_Y, Center_bullet_worry_X + 20, Center_bullet_worry_Y - 20);
                UILineDraw(&Bullet_worry3, "BW3", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X + 40, Center_bullet_worry_Y, Center_bullet_worry_X + 20, Center_bullet_worry_Y + 20);
            }
            if (tick_num_bulletUI < 5) {
                UILineDraw(&Bullet_worry1, "BW1", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X - 40, Center_bullet_worry_Y, Center_bullet_worry_X + 40, Center_bullet_worry_Y);
                UILineDraw(&Bullet_worry2, "BW2", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X + 40, Center_bullet_worry_Y, Center_bullet_worry_X + 20, Center_bullet_worry_Y - 20);
                UILineDraw(&Bullet_worry3, "BW3", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 4, Center_bullet_worry_X + 40, Center_bullet_worry_Y, Center_bullet_worry_X + 20, Center_bullet_worry_Y + 20);
            }
            tick_num_bulletUI %= 10;
    }
    if (UI_Now->Shoot_enemy == 1) {
        UILineDraw(&Shoot_enemy1, "SE1", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 4, SCREEN_LENGTH / 2 + 40, SCREEN_WIDTH / 2 + 40, SCREEN_LENGTH / 2 - 40, SCREEN_WIDTH / 2 - 40);
        UILineDraw(&Shoot_enemy2, "SE2", Graphic_Operate_ADD, 5, Graphic_Color_Purplish_red, 4, SCREEN_LENGTH / 2 - 40, SCREEN_WIDTH / 2 + 40, SCREEN_LENGTH / 2 + 40, SCREEN_WIDTH / 2 - 40);
    } else {
        UILineDraw(&Shoot_enemy1, "SE1", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 1, SCREEN_LENGTH + 80, SCREEN_WIDTH + 80, SCREEN_LENGTH - 80, SCREEN_WIDTH - 80);
        UILineDraw(&Shoot_enemy2, "SE2", Graphic_Operate_DEL, 5, Graphic_Color_Purplish_red, 1, SCREEN_LENGTH - 80, SCREEN_WIDTH + 80, SCREEN_LENGTH + 80, SCREEN_WIDTH - 80);
    }
    UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Shoot_enemy1, Shoot_enemy2, Bullet_worry1, Bullet_worry2, Bullet_worry3);
    UIGraphRefresh(&referee_data_for_ui->referee_id, 1, MAX_POWER);

    UIFloatDraw(&CAP_power, "CAP", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 0, 2, SCREEN_LENGTH / 2, SCREEN_WIDTH / 2 - 300, UI_Now->CapVot * 100);
    UIFloatDraw(&Pitch_angle, "PTD", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 30, 0, 2, SCREEN_LENGTH / 2 + 100, SCREEN_WIDTH / 2, UI_Now->pitch_data * 100);
    UICharRefresh(&referee_data_for_ui->referee_id, friction_mode);
    UICharRefresh(&referee_data_for_ui->referee_id, rotate_mode);
    UIGraphRefresh(&referee_data_for_ui->referee_id, 7, Pitch_angle, CAP_power, Friction_speed, Dirction_Chassis4, Dirction_Chassis3, Dirction_Chassis2, Dirction_Chassis1);
    UI_last = *UI_Now;
}

// void UI_dirction_draw()
// {
//     if (init_flag == 1) {
//         UILineDraw(&Pitch_dirction, "PD3", Graphic_Operate_ADD, 8, Graphic_Color_Yellow, 2, Pitch_angle_X,
//                    Pitch_angle_Y + 40, Pitch_angle_X + 60 * arm_cos_f32(-UI_last.pitch_data * DEGREE_2_RAD),
//                    Pitch_angle_Y + 40 + 60 * arm_sin_f32(UI_last.pitch_data * DEGREE_2_RAD));
//         UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Pitch_dirction);
//         UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
//                      Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45) * DEGREE_2_RAD),
//                      Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45) * DEGREE_2_RAD), 30);
//         UICircleDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_ADD, 9, Graphic_Color_Pink, 2,
//                      Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 90) * DEGREE_2_RAD),
//                      Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 90) * DEGREE_2_RAD), 30);
//         UICircleDraw(&Dirction_Chassis3, "CD3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 2,
//                      Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 180) * DEGREE_2_RAD),
//                      Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 180) * DEGREE_2_RAD), 30);
//         UICircleDraw(&Dirction_Chassis4, "CD4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 2,
//                      Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 270) * DEGREE_2_RAD),
//                      Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 270) * DEGREE_2_RAD), 30);
//         UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Dirction_Chassis1, Dirction_Chassis2);
//         UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Dirction_Chassis3, Dirction_Chassis4);

//     } else {
//         UILineDraw(&Pitch_dirction, "PD3", Graphic_Operate_CHANGE, 8, Graphic_Color_Yellow, 2, Pitch_angle_X,
//                    Pitch_angle_Y + 40, Pitch_angle_X + 60 * arm_cos_f32(-UI_last.pitch_data * DEGREE_2_RAD),
//                    Pitch_angle_Y + 40 + 60 * arm_sin_f32(UI_last.pitch_data * DEGREE_2_RAD));
//         UIGraphRefresh(&referee_data_for_ui->referee_id, 1, Pitch_dirction);
//         UICircleDraw(&Dirction_Chassis1, "CD1", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 2,
//                      Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45) * DEGREE_2_RAD),
//                      Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45) * DEGREE_2_RAD), 30);
//         UICircleDraw(&Dirction_Chassis2, "CD2", Graphic_Operate_CHANGE, 9, Graphic_Color_Pink, 2,
//                      Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 90) * DEGREE_2_RAD),
//                      Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 90) * DEGREE_2_RAD), 30);
//         UICircleDraw(&Dirction_Chassis3, "CD3", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 2,
//                      Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 180) * DEGREE_2_RAD),
//                      Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 180) * DEGREE_2_RAD), 30);
//         UICircleDraw(&Dirction_Chassis4, "CD4", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 2,
//                      Center_Of_Dirction_X + Len_Dirction * arm_cos_f32((-UI_last.Angle + 45 + 270) * DEGREE_2_RAD),
//                      Center_Of_Dirction_Y + Len_Dirction * arm_sin_f32((-UI_last.Angle + 45 + 270) * DEGREE_2_RAD), 30);
//         UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Dirction_Chassis1, Dirction_Chassis2);
//         UIGraphRefresh(&referee_data_for_ui->referee_id, 2, Dirction_Chassis3, Dirction_Chassis4);
//     }
// }
// void UI_ALL_Robot_HP()
// {
//     if (referee_data_for_ui->referee_id.Robot_Color == Robot_Blue) {
//         if (init_flag == 1) {

//             UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 400, UI_last.All_robot_HP.red_1_robot_HP);
//             UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 440, UI_last.All_robot_HP.red_2_robot_HP);
//             UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 480, UI_last.All_robot_HP.red_3_robot_HP);
//             UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 520, UI_last.All_robot_HP.red_4_robot_HP);
//             UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 560, UI_last.All_robot_HP.red_5_robot_HP);
//             UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 600, UI_last.All_robot_HP.red_7_robot_HP);
//             UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
//             UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
//         } else {
//             UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 400, UI_last.All_robot_HP.red_1_robot_HP);
//             UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 440, UI_last.All_robot_HP.red_2_robot_HP);
//             UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 480, UI_last.All_robot_HP.red_3_robot_HP);
//             UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 520, UI_last.All_robot_HP.red_4_robot_HP);
//             UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 560, UI_last.All_robot_HP.red_5_robot_HP);
//             UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 600, UI_last.All_robot_HP.red_7_robot_HP);
//             UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
//             UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
//         }
//     } else {
//         if (init_flag == 1) {
//             UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 400, UI_last.All_robot_HP.blue_1_robot_HP);
//             UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 440, UI_last.All_robot_HP.blue_2_robot_HP);
//             UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 480, UI_last.All_robot_HP.blue_3_robot_HP);
//             UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 520, UI_last.All_robot_HP.blue_4_robot_HP);
//             UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 560, UI_last.All_robot_HP.blue_5_robot_HP);
//             UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_ADD, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 600, UI_last.All_robot_HP.blue_7_robot_HP);
//             UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
//             UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
//         } else {
//             UIIntDraw(&Robot1HP, "HP1", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 400, UI_last.All_robot_HP.blue_1_robot_HP);
//             UIIntDraw(&Robot2HP, "HP2", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 440, UI_last.All_robot_HP.blue_2_robot_HP);
//             UIIntDraw(&Robot3HP, "HP3", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 480, UI_last.All_robot_HP.blue_3_robot_HP);
//             UIIntDraw(&Robot4HP, "HP4", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 520, UI_last.All_robot_HP.blue_4_robot_HP);
//             UIIntDraw(&Robot5HP, "HP5", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 560, UI_last.All_robot_HP.blue_5_robot_HP);
//             UIIntDraw(&RobotsentryHP, "HP6", Graphic_Operate_CHANGE, 9, Graphic_Color_Yellow, 20, 2, SCREEN_LENGTH - 340, SCREEN_WIDTH - 600, UI_last.All_robot_HP.blue_7_robot_HP);
//             UIGraphRefresh(&referee_data_for_ui->referee_id, 5, Robot1HP, Robot2HP, Robot3HP, Robot4HP, Robot5HP);
//             UIGraphRefresh(&referee_data_for_ui->referee_id, 1, RobotsentryHP);
//         }
//     }
// }