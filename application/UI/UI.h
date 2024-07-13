#ifndef UI_H
#define UI_H

#include "referee_UI.h"
typedef enum {
    LINE=0,
    Circle,
    Retangle,
    ARC,
    OVAL,
    INT_TYPE,
    FLOAT_TYPE,
} UI_type_e;
typedef struct 
{
    Graph_Data_t UI_instantiation;
    char name[3];
    int8_t Color;
    int8_t Layer;
    int8_t Width;
    int8_t Size;
    UI_type_e TYPE;
    int16_t coordinate_axis[4];
    int16_t radius;
    uint8_t init_flag;
    int INT_NUM;
    double DOUBLE_NUM;
    /* data */
}UI_Graph_Instance;
typedef struct 
{  
    String_Data_t UI_instantiation;
    char name[3];
    int8_t Color;
    int8_t Layer;
    int8_t Width;
    int8_t Size;
    UI_type_e TYPE;
    int16_t coordinate_axis[4];
    int16_t radius;
    /* data */
}UI_Char_Instance;



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
extern UI_Graph_Instance *UI_LINE_Init(char Name[3], int8_t color, int8_t layer, int8_t width, int16_t startx, int16_t starty, int16_t endx, int16_t endy);
extern void UI_DEBUG_MODE(uint8_t CNT,int px,int py);
extern void MyUIInit(void);
extern void MyUIRefresh(void);
extern void get_referee_data(referee_info_t *referee_data);
extern uint8_t Change_UI_Data(UIdate_for_change *UI_now);
extern void UIfresh_Always();
extern int UI_flag_second;
#endif