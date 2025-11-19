#include "stm32f10x.h"
#include "motor.h"
#include "Delay.h"  
#include "OLED.h"
#include "SPP_serial.h"
#include "servo.h"
#include "HCSR04.h"
#include "HCSR04_Follow.h"
#include "HCSR04_AO.h"

// 工作模式枚举
typedef enum {
    MODE_MANUAL = 0,
    MODE_FOLLOW,
    MODE_OBSTACLE_AVOID
} WorkMode;

int main(void)
{
    /* 系统初始化 */
    SystemInit();        // 系统时钟初始化
    Delay_ms(100);       // 系统延时，确保稳定
    OLED_Init();         // OLED初始化   
    Delay_ms(100);       // OLED初始化延时
    Motor_Init();        // 电机初始化    
    Serial_Init();       // 蓝牙串口初始化    
    Servo_Init();        // 舵机初始化    
    HCSR04_Init();       // 超声波初始化
    HCSR04_Follow_Init(); // 跟随模块初始化
    HCSR04_AO_Init();    // 避障模块初始化
    
    // 工作模式变量
    static WorkMode current_mode = MODE_MANUAL;  // 当前工作模式：手动、跟随或避障
    static uint32_t distance = 0;                // 当前测量的距离值，单位：cm
    
    // 初始化显示
    OLED_Clear();
    OLED_ShowString(1, 1, "Mode: Manual    ");
	OLED_ShowString(2, 1, "Dist:           ");
    OLED_ShowString(4, 1, "Ready           ");
    Delay_ms(500);
    
    // 外部函数声明
    extern uint8_t HCSR04_Follow_IsRunning(void);
    extern uint8_t HCSR04_AO_IsRunning(void);
    
    while(1)
    {
        // 蓝牙控制处理（所有模式下都处理蓝牙指令）
        spp_control();
        
        HCSR04_DisplayDistance();
		distance = HCSR04_GetDistance();

        
        // 模式切换检查 - 根据模块运行状态更新当前模式
        if (HCSR04_Follow_IsRunning()) {
            current_mode = MODE_FOLLOW;
			HCSR04_Follow_Update(distance);
			OLED_ShowString(1, 1, "Mode: Follow    ");
        } else if (HCSR04_AO_IsRunning()) {
            current_mode = MODE_OBSTACLE_AVOID;
			HCSR04_AO_Update();
			OLED_ShowString(1, 1, "Mode: Avoid     ");
        } else {
            // 手动模式
            current_mode = MODE_MANUAL;
			OLED_ShowString(1, 1, "Mode: Manual    ");
        }
 
        Delay_ms(100);    // 主循环延时
    }
}
