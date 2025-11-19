#include "stm32f10x.h"                  // Device header
#include "motor.h"

void Motor_Init(void)
{
    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 初始化方向控制引脚 (PA4, PA5, PA6, PA7)
	GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = LEFT_AIN1_PIN | LEFT_AIN2_PIN | 
                                  RIGHT_BIN1_PIN | RIGHT_BIN2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始化PWM引脚 (PA2, PA3) - 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = LEFT_PWM_PIN | RIGHT_PWM_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // TIM2时基单元初始化
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler = 3;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 初始化PWM输出比较通道
	TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 0;  // 初始占空比为0
    
    // 通道3 - PA2 (左边电机组PWM)
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // 通道4 - PA3 (右边电机组PWM)  
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // 使能定时器预装载
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
    
    // 初始停止所有电机
    Motor_StopAll();
}

void Motor_SetDirection(Motor_Side side, Motor_Direction dir)
{
    switch(side) {
        case MOTOR_LEFT:
            switch(dir) {
                case MOTOR_STOP:
                case MOTOR_BRAKE:
                    GPIO_ResetBits(LEFT_AIN1_PORT, LEFT_AIN1_PIN);
                    GPIO_ResetBits(LEFT_AIN2_PORT, LEFT_AIN2_PIN);
                    break;
                case MOTOR_FORWARD:
                    GPIO_SetBits(LEFT_AIN1_PORT, LEFT_AIN1_PIN);
                    GPIO_ResetBits(LEFT_AIN2_PORT, LEFT_AIN2_PIN);
                    break;
                case MOTOR_BACKWARD:
                    GPIO_ResetBits(LEFT_AIN1_PORT, LEFT_AIN1_PIN);
                    GPIO_SetBits(LEFT_AIN2_PORT, LEFT_AIN2_PIN);
                    break;
            }
            break;
            
        case MOTOR_RIGHT:
            switch(dir) {
                case MOTOR_STOP:
                case MOTOR_BRAKE:
                    GPIO_ResetBits(RIGHT_BIN1_PORT, RIGHT_BIN1_PIN);
                    GPIO_ResetBits(RIGHT_BIN2_PORT, RIGHT_BIN2_PIN);
                    break;
                case MOTOR_FORWARD:
                    GPIO_SetBits(RIGHT_BIN1_PORT, RIGHT_BIN1_PIN);
                    GPIO_ResetBits(RIGHT_BIN2_PORT, RIGHT_BIN2_PIN);
                    break;
                case MOTOR_BACKWARD:
                    GPIO_ResetBits(RIGHT_BIN1_PORT, RIGHT_BIN1_PIN);
                    GPIO_SetBits(RIGHT_BIN2_PORT, RIGHT_BIN2_PIN);
                    break;
            }
            break;
    }
}

void Motor_SetSpeed(Motor_Side side, int16_t speed)
{
    // 限制速度范围 -1000 ~ 1000
    if(speed > 1000) speed = 1000;
    if(speed < -1000) speed = -1000;
    
    // 设置方向和PWM
    if(speed == 0) {
        Motor_SetDirection(side, MOTOR_STOP);
    } else if(speed > 0) {
        Motor_SetDirection(side, MOTOR_FORWARD);
    } else {
        Motor_SetDirection(side, MOTOR_BACKWARD);
        speed = -speed;  // 取绝对值
    }
    
    // 设置PWM占空比 (速度映射到PWM值)
    uint16_t pwm_value = (speed * PWM_PERIOD) / 1000;
    
    switch(side) {
        case MOTOR_LEFT:
            TIM_SetCompare3(TIM2, pwm_value);
            break;
        case MOTOR_RIGHT:
            TIM_SetCompare4(TIM2, pwm_value);
            break;
    }
}

void Motor_SetLeftRight(int16_t left_speed, int16_t right_speed)
{
    // 同时设置左右电机速度
    Motor_SetSpeed(MOTOR_LEFT, left_speed);
    Motor_SetSpeed(MOTOR_RIGHT, right_speed);
}

void Motor_Stop(Motor_Side side)
{
    Motor_SetDirection(side, MOTOR_STOP);
    switch(side) {
        case MOTOR_LEFT:
            TIM_SetCompare3(TIM2, 0);
            break;
        case MOTOR_RIGHT:
            TIM_SetCompare4(TIM2, 0);
            break;
    }
}

void Motor_Brake(Motor_Side side)
{
    Motor_SetDirection(side, MOTOR_BRAKE);
    switch(side) {
        case MOTOR_LEFT:
            TIM_SetCompare3(TIM2, PWM_PERIOD);  // 全占空比刹车
            break;
        case MOTOR_RIGHT:
            TIM_SetCompare4(TIM2, PWM_PERIOD);
            break;
    }
}

void Motor_StopAll(void)
{
    Motor_Stop(MOTOR_LEFT);
    Motor_Stop(MOTOR_RIGHT);
}