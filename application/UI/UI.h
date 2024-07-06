#ifndef UI_H
#define UI_H

#include "referee_UI.h"
extern void MyUIInitOperate(void);

typedef struct
{
    int remain_HP;
    int Max_HP;
    friction_mode_e fri_mode;
    chassis_mode_e chassis_mode;
    float pitch_data;
    uint8_t load_Mode;
    float Angle;
    uint8_t Bullet_ready;
    ext_game_robot_HP_t All_robot_HP;
    float CapVot;
    int Frition_speed;
    uint8_t Shoot_enemy;
    uint8_t Bullet_empty;
    //Shoot_Step Air_ready;
}UIdate_for_change;
extern Graph_Data_t line_fuzhu_one;
extern Graph_Data_t line_fuzhu_two;
extern Graph_Data_t line_fuzhu_three;
extern void MyUIInit(void);
extern void MyUIRefresh(void);
extern void get_referee_data(referee_info_t *referee_data);
extern uint8_t Change_UI_Data(UIdate_for_change *UI_now);
extern void UIfresh_Always();
extern int UI_flag_second;
#endif