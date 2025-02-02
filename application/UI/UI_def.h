#ifndef UI_DEF_H
#define UI_DEF_H

//屏幕宽度
#define SCREEN_WIDTH 1080
//屏幕长度
#define SCREEN_LENGTH 1920

//图形处理方式
#define Graphic_Operate_ADD 1
#define Graphic_Operate_CHANGE 2
#define Graphic_Operate_DEL 3

//图形类型
#define Graphic_Type_LINE 0
#define Graphic_Type_RECT 1
#define Graphic_Type_CIRCLE 2
#define Graphic_Type_FLOAT 5
#define Graphic_Type_INT 6
#define Graphic_Type_CHAR 7

//图形颜色
#define Graphic_Color_Main 0            //不知道
#define Graphic_Color_Yellow 1			//黄色
#define Graphic_Color_Green 2			//绿色
#define Graphic_Color_Orange 3			//橙色
#define Graphic_Color_Purplish_red 4    //红色
#define Graphic_Color_Pink 5		    //粉色
#define Graphic_Color_Cyan 6            //不知道
#define Graphic_Color_Black 7			//黑色
#define Graphic_Color_White 8			//白色

//Other
#define Center_Of_Dirction_X (SCREEN_LENGTH/2+100)
#define Center_Of_Dirction_Y (SCREEN_WIDTH/2-80)
#define Len_Dirction (40)

#define Pitch_angle_X (SCREEN_LENGTH/2+10)
#define Pitch_angle_Y (SCREEN_WIDTH/2)
#define Radius_Pitch 80

#define center_tigger_X (SCREEN_LENGTH/2-30)
#define center_tigger_Y (SCREEN_WIDTH/2-40)

#endif