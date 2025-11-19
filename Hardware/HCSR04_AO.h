#ifndef __HCSR04_AO_H
#define __HCSR04_AO_H

#include "stm32f10x.h"
#include "HCSR04.h"
#include "car_control.h"
#include "servo.h"

// 避障模式参数
#define OBSTACLE_DISTANCE_THRESHOLD 30  // 障碍物检测阈值30cm
#define TURN_SPEED 250                  // 转弯速度

// 避障状态枚举
typedef enum {
    AO_STATE_FORWARD = 0,          // 前进状态
    AO_STATE_CHECK_LEFT,           // 检查左侧
    AO_STATE_CHECK_RIGHT,          // 检查右侧
    AO_STATE_TURN_LEFT,            // 左转
    AO_STATE_TURN_RIGHT,           // 右转
    AO_STATE_BACKWARD              // 后退
} AO_State;

// 函数声明
void HCSR04_AO_Init(void);
void HCSR04_AO_Update(void);
void HCSR04_AO_Start(void);
void HCSR04_AO_Stop(void);
uint8_t HCSR04_AO_IsRunning(void);
uint8_t HCSR04_CheckDirection(int16_t angle);

#endif
