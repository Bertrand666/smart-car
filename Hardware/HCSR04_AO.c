#include "HCSR04_AO.h"
#include "OLED.h"
#include "Delay.h"
#include "stm32f10x_tim.h"  // 添加定时器头文件
#include "servo.h"

// 避障模式状态变量
static uint8_t ao_enabled = 0;                    // 避障模式使能标志：0-禁用，1-启用
static AO_State current_state = AO_STATE_FORWARD_CHECK; // 当前避障状态机状态
static uint8_t obstacle_front = 0;                // 前方障碍物标志：0-无障碍物，1-有障碍物
static uint8_t obstacle_left = 0;                 // 左侧障碍物标志：0-无障碍物，1-有障碍物
static uint8_t obstacle_right = 0;                // 右侧障碍物标志：0-无障碍物，1-有障碍物

void HCSR04_AO_Init(void)
{
    ao_enabled = 0;
    current_state = AO_STATE_FORWARD_CHECK;
    obstacle_front = 0;
    obstacle_left = 0;
    obstacle_right = 0;
}

uint32_t HCSR04_CheckDirection(int16_t angle)
{
    // 设置舵机角度
    Servo_SetAngle(angle);
    Delay_ms(500); // 等待舵机转动稳定
    
    // 测量距离
    uint32_t distance = HCSR04_GetDistance();
    
    // 判断是否有障碍物
    if (distance > 0 && distance <= OBSTACLE_DISTANCE_THRESHOLD) {
        return 1; // 有障碍物
    } else {
        return 0; // 无障碍物
    }
}

void HCSR04_AO_Update(void)
{
    if (!ao_enabled) return;
    
    switch (current_state) {
        case AO_STATE_FORWARD_CHECK:
            // 检查前方障碍物（舵机在0度位置）
            obstacle_front = HCSR04_CheckDirection(0); 
            if (!obstacle_front) {
                // 前方无障碍物，前进
                Car_Forward(200);
                OLED_ShowString(4, 1, "Moving Forward");
            } else {
                // 前方有障碍物，停止并准备检查左侧
                Car_Stop();
                OLED_ShowString(4, 1, "Front Blocked ");
                // 重置左右障碍物标志，准备新的检测
                obstacle_left = 0;
                obstacle_right = 0;
                current_state = AO_STATE_LEFT_CHECK;
            }
            break;
            
        case AO_STATE_LEFT_CHECK:
            // 显示正在检查左侧
            OLED_ShowString(4, 1, "Checking Left  ");
            // 旋转舵机到90度位置并检查左侧障碍物
            obstacle_left = HCSR04_CheckDirection(90);
			Delay_ms(100);
            
            if (!obstacle_left) {
                // 左侧无障碍物，准备左转
                OLED_ShowString(4, 1, "Left Clear    ");
                current_state = AO_STATE_TURN_LEFT;
            } else {
                // 左侧有障碍物，准备检查右侧
                OLED_ShowString(4, 1, "Left Blocked  ");
                current_state = AO_STATE_RIGHT_CHECK;
            }
            break;
            
        case AO_STATE_RIGHT_CHECK:
            // 显示正在检查右侧
            OLED_ShowString(4, 1, "Checking Right ");
            // 旋转舵机到-90度位置并检查右侧障碍物
            obstacle_right = HCSR04_CheckDirection(-90);
			Delay_ms(100);
            
            if (!obstacle_right) {
                // 右侧无障碍物，准备右转
                OLED_ShowString(4, 1, "Right Clear   ");
                current_state = AO_STATE_TURN_RIGHT;
            } else {
                // 左右都有障碍物，停止并显示"Many obstacles"
                Servo_SetAngle(0); // 舵机复位到0度
                Car_Stop();
                OLED_ShowString(4, 1, "Many obstacles");
                current_state = AO_STATE_STOP;
            }
            break;
            
        case AO_STATE_TURN_LEFT:
            // 显示正在左转
            OLED_ShowString(4, 1, "Turning Left  ");
            // 左转90度
            Car_SpinLeft(300);
			Delay_ms(700);
			Car_Forward(300);
            
            // 转弯完成，恢复舵机角度，继续前进检测
            Servo_SetAngle(0);
            Car_Stop();
            Delay_ms(500); // 等待稳定
            
            // 回到前方检测状态
            current_state = AO_STATE_FORWARD_CHECK;
            break;
            
        case AO_STATE_TURN_RIGHT:
            // 显示正在右转
            OLED_ShowString(4, 1, "Turning Right ");
            // 右转90度
            Car_SpinRight(300);
			Delay_ms(700);
			Car_Forward(300);
            
            // 转弯完成，恢复舵机角度，继续前进检测
            Servo_SetAngle(0);
            Car_Stop();
            Delay_ms(500); // 等待稳定
            
            // 回到前方检测状态
            current_state = AO_STATE_FORWARD_CHECK;
            break;
            
        case AO_STATE_STOP:
            // 停止状态，保持停止
            Car_Stop();
            OLED_ShowString(4, 1, "Many obstacles");
            break;
    }
}

void HCSR04_AO_Start(void)
{
    ao_enabled = 1;
    current_state = AO_STATE_FORWARD_CHECK;
    Servo_SetAngle(0); // 确保舵机在初始位置
    OLED_ShowString(1, 1, "AO Mode: ON    ");
}

void HCSR04_AO_Stop(void)
{
    ao_enabled = 0;
    Car_Stop();
    Servo_SetAngle(0); // 恢复舵机初始位置
    OLED_ShowString(1, 1, "AO Mode: OFF   ");
    OLED_ShowString(4, 1, "                ");
}

uint8_t HCSR04_AO_IsRunning(void)
{
    return ao_enabled;
}
