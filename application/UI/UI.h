#ifndef UI_H
#define UI_H

#include "referee_UI.h"
extern void MyUIInitOperate(void);

typedef struct
{
    int remain_HP;
    int Max_HP;
    uint8_t fir_mode;
    chassis_mode_e chassis_mode;
    int pitch_data;
    uint8_t load_Mode;
    float Angle;
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