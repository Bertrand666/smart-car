#ifndef __MOTOR_H
#define __MOTOR_H

// PWM周期值 (决定PWM频率)
#define PWM_PERIOD 999  // 72MHz / (999+1) = 72kHz PWM频率

// GPIO引脚定义
// 左边电机组控制
#define LEFT_PWM_PIN     GPIO_Pin_2    // PA2 - TIM2_CH3
#define LEFT_PWM_PORT    GPIOA
#define LEFT_AIN1_PIN    GPIO_Pin_4    // PA4
#define LEFT_AIN2_PIN    GPIO_Pin_5    // PA5
#define LEFT_AIN1_PORT   GPIOA
#define LEFT_AIN2_PORT   GPIOA

// 右边电机组控制  
#define RIGHT_PWM_PIN    GPIO_Pin_3    // PA3 - TIM2_CH4
#define RIGHT_PWM_PORT   GPIOA
#define RIGHT_BIN1_PIN   GPIO_Pin_6    // PA6
#define RIGHT_BIN2_PIN   GPIO_Pin_7    // PA7
#define RIGHT_BIN1_PORT  GPIOA
#define RIGHT_BIN2_PORT  GPIOA


// 电机组定义
typedef enum {
    MOTOR_LEFT = 0,     // 左边两个电机（并联）
    MOTOR_RIGHT         // 右边两个电机（并联）
} Motor_Side;

// 电机方向定义
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_BRAKE
} Motor_Direction;

// 函数声明
void Motor_Init(void);
void Motor_SetSpeed(Motor_Side side, int16_t speed);
void Motor_Stop(Motor_Side side);
void Motor_Brake(Motor_Side side);
void Motor_StopAll(void);
void Motor_SetLeftRight(int16_t left_speed, int16_t right_speed);

#endif