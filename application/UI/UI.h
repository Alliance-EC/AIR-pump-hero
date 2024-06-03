#ifndef UI_H
#define UI_H

#include "referee_UI.h"
extern void MyUIInitOperate(void);

typedef struct
{
    int remain_HP;
    int Max_HP;
<<<<<<< HEAD
    uint8_t shoot_mode;
=======
    uint8_t fir_mode;
>>>>>>> 82775a6cbbda179c12e42b923b2e54f64f9d7f72
    chassis_mode_e chassis_mode;
    float pitch_data;
    uint8_t load_Mode;
    float Angle;
<<<<<<< HEAD
    uint8_t Bullet_ready;
    ext_game_robot_HP_t All_robot_HP;
    float CapVot;
    //Shoot_Step Air_ready;
=======
    ext_game_robot_HP_t All_robot_HP;
    float CapVot;
>>>>>>> 82775a6cbbda179c12e42b923b2e54f64f9d7f72
}UIdate_for_change;
extern Graph_Data_t line_fuzhu_one;
extern Graph_Data_t line_fuzhu_two;
extern Graph_Data_t line_fuzhu_three;
extern void MyUIInit(void);
extern void MyUIRefresh(void);
extern void get_referee_data(referee_info_t *referee_data);
extern uint8_t check_to_change_UI(UIdate_for_change *UI_now);
extern void UIfresh_Always();
#endif