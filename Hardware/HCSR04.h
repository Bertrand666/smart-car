#ifndef __HCSR04_H
#define __HCSR04_H

// 超声波引脚定义
#define ULTRASONIC_TRIG_PIN    GPIO_Pin_1   // PA1作为Trig引脚
#define ULTRASONIC_TRIG_PORT   GPIOA
#define ULTRASONIC_ECHO_PIN    GPIO_Pin_11  // PA11作为Echo引脚(TIM1_CH4)
#define ULTRASONIC_ECHO_PORT   GPIOA

// 参数定义
#define SOUND_SPEED           34000       // 声速340m/s = 34000cm/s
#define ULTRASONIC_TIMEOUT    100000      // 超时值(对应100ms)

// 函数声明
void HCSR04_Init(void);
uint32_t HCSR04_GetDistance(void);
void HCSR04_DisplayDistance(uint32_t distance);

#endif
