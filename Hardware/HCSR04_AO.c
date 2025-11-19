#include "HCSR04_AO.h"
#include "OLED.h"
#include "Delay.h"
#include "servo.h"
#include "SPP_serial.h"

// 避障模式状态变量
static uint8_t ao_enabled = 0;                    // 避障模式使能标志：0-禁用，1-启用
static AO_State current_state = AO_STATE_FORWARD; // 当前避障状态机状态
static uint8_t left_clear = 0;                    // 左侧是否无障碍物
static uint8_t right_clear = 0;                   // 右侧是否无障碍物

void HCSR04_AO_Init(void)
{
    ao_enabled = 0;
    current_state = AO_STATE_FORWARD;
    left_clear = 0;
    right_clear = 0;
}

/**
  * @brief  检查指定方向的障碍物
  * @param  angle: 舵机角度
  * @retval 1-无障碍物，0-有障碍物
  */

void HCSR04_AO_Update(void)
{
    if (!ao_enabled) return;
	
	uint32_t distance;
    
    switch (current_state) {
        case AO_STATE_FORWARD:
            // 设置舵机到正前方并直接测量距离
            Servo_SetAngle(0);
		
			// 测量距离
			distance = HCSR04_GetDistance();
			Serial_SendNumber(distance, 3);
		
            // 直接判断前方障碍物
            if (distance > 0 && distance <= OBSTACLE_DISTANCE_THRESHOLD) {
                // 检测到障碍物，立即停止
                Car_Stop();
                current_state = AO_STATE_CHECK_LEFT;
            } else {
                // 无障碍物，继续前进
                Car_Forward(200);
				OLED_ShowString(4, 1, "                ");
                OLED_ShowString(4, 1, "Moving Forward  ");
            }
            break;
            
        case AO_STATE_CHECK_LEFT:
            
            // 舵机左转90度并检查左侧
			Servo_SetAngle(90);
			Delay_ms(1000);
			
			// 测量距离
			distance = HCSR04_GetDistance();
			Serial_SendNumber(distance, 3);
		
			left_clear = (distance < OBSTACLE_DISTANCE_THRESHOLD) ? 1 : 0;
		
			if (!left_clear) {
				// 左侧无障碍物，准备左转	
				current_state = AO_STATE_TURN_LEFT;
			} else {
				// 左侧有障碍物，准备检查右侧				
				current_state = AO_STATE_CHECK_RIGHT;
			}	
            break;
            
        case AO_STATE_CHECK_RIGHT:
 
			// 舵机右转90度并检查侧
			Servo_SetAngle(-90);
			Delay_ms(1000);
		
			// 测量距离
			distance = HCSR04_GetDistance();
			Serial_SendNumber(distance, 3);
			
			right_clear = (distance < OBSTACLE_DISTANCE_THRESHOLD) ? 1 : 0;
			
			if (!right_clear) {
				// 右侧无障碍物，准备右转
				current_state = AO_STATE_TURN_RIGHT;
			} else {
				// 两侧都有障碍物，后退
				current_state = AO_STATE_BACKWARD;
			}
            
            break;
            
        case AO_STATE_TURN_LEFT:
			
			// 左转后直行
			Car_SpinLeft(300);
			Delay_ms(700);
			Car_Forward(300);
		
			// 舵机复位到0度
			Servo_SetAngle(0);
			Delay_ms(200);
		
			current_state = AO_STATE_FORWARD;
 
            break;
            
        case AO_STATE_TURN_RIGHT:
            // 右转后直行
			Car_SpinRight(300);
			Delay_ms(700);
			Car_Forward(300);
		
			// 舵机复位到0度
			Servo_SetAngle(0);
			Delay_ms(200);
			
			current_state = AO_STATE_FORWARD;
			
 
            break;
            
        case AO_STATE_BACKWARD:

			// 舵机复位到0度
            Servo_SetAngle(0);
            Delay_ms(200);
            
            Car_Backward(200);
			Delay_ms(1000);
		
			current_state = AO_STATE_FORWARD;
   
            break;
    }
    
}

void HCSR04_AO_Start(void)
{
    ao_enabled = 1;
    current_state = AO_STATE_FORWARD;
    left_clear = 0;
    right_clear = 0;
    Servo_SetAngle(0); // 确保舵机在初始位置
}

void HCSR04_AO_Stop(void)
{
    ao_enabled = 0;
    Car_Stop();
    Servo_SetAngle(0); // 恢复舵机初始位置
    OLED_ShowString(4, 1, "                ");
}

uint8_t HCSR04_AO_IsRunning(void)
{
    return ao_enabled;
}
