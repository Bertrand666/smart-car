#ifndef __HCSR04_FOLLOW_H
#define __HCSR04_FOLLOW_H

#include "stm32f10x.h"
#include "HCSR04.h"
#include "car_control.h"
#include "servo.h"

// 跟随模式参数
#define FOLLOW_DISTANCE_THRESHOLD 10  // 跟随距离阈值10cm
#define FOLLOW_SPEED 300              // 跟随速度

// 函数声明
void HCSR04_Follow_Init(void);
void HCSR04_Follow_Update();
void HCSR04_Follow_Start(void);
void HCSR04_Follow_Stop(void);
uint8_t HCSR04_Follow_IsRunning(void);

#endif
