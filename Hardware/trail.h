#ifndef __TRAIL_H
#define __TRAIL_H

#include "stm32f10x.h"
#include "car_control.h"

// 红外循迹模块引脚定义
#define TRAIL_SENSOR_LEFT     GPIO_Pin_11   // PB11 - 最左侧传感器
#define TRAIL_SENSOR_LEFT_MID GPIO_Pin_10   // PB10 - 左侧中间传感器  
#define TRAIL_SENSOR_RIGHT_MID GPIO_Pin_13  // PB13 - 右侧中间传感器
#define TRAIL_SENSOR_RIGHT    GPIO_Pin_12   // PB12 - 最右侧传感器
#define TRAIL_SENSOR_PORT     GPIOB

// 循迹模式参数
#define TRAIL_SPEED 400                   // 循迹速度
#define TRAIL_TURN_SPEED 200              // 转弯速度

// 循迹模式状态变量
extern uint8_t trail_enabled;             // 循迹模式使能标志

// 函数声明
void Trail_Init(void);
void Trail_Update(void);
void Trail_Start(void);
void Trail_Stop(void);
uint8_t Trail_IsRunning(void);

#endif
