#include "stm32f10x.h"                  // Device header

#include "motor.h"
#include "Delay.h"

/* 小车运动控制函数 */

// 前进
void Car_Forward(uint16_t speed)
{
    Motor_SetLeftRight(speed, speed);
}

// 后退
void Car_Backward(uint16_t speed)
{
    Motor_SetLeftRight(-speed, -speed);
}

// 左转弯
void Car_TurnLeft(uint16_t speed)
{
    Motor_SetLeftRight(0, speed);
}

// 右转弯
void Car_TurnRight(uint16_t speed)
{
    Motor_SetLeftRight(speed, 0);
}

// 原地左转：左轮反转，右轮正转
void Car_SpinLeft(uint16_t speed)
{
    
    Motor_SetLeftRight(-speed, speed);
}

// 原地右转：左轮正转，右轮反转
void Car_SpinRight(uint16_t speed)
{  
    Motor_SetLeftRight(speed, -speed);
}

// 停车
void Car_Stop(void)
{
    Motor_StopAll();
}

// 紧急刹车
void Car_Brake(void)
{
    Motor_Brake(MOTOR_LEFT);
    Motor_Brake(MOTOR_RIGHT);
}

// 渐进加速
void Car_Accelerate(uint16_t target_speed, uint16_t steps)
{
    uint16_t i;
    for(i = 0; i < target_speed; i += steps) {
        Car_Forward(i);
        Delay_ms(50);
    }
    Car_Forward(target_speed);
}
