#include "trail.h"
#include "OLED.h"
#include "Delay.h"

// 循迹模式状态变量
uint8_t trail_enabled = 0;        // 循迹模式使能标志：0-禁用，1-启用

void Trail_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 初始化红外循迹传感器引脚为输入模式
    GPIO_InitStructure.GPIO_Pin = TRAIL_SENSOR_LEFT | TRAIL_SENSOR_LEFT_MID | 
                                  TRAIL_SENSOR_RIGHT_MID | TRAIL_SENSOR_RIGHT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入，默认高电平
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TRAIL_SENSOR_PORT, &GPIO_InitStructure);
    
    trail_enabled = 0;
}

// 循迹控制函数
void Trail_Update(void)
{
    if (!trail_enabled) return;
    
    // 读取四个红外传感器的状态
    uint8_t L = GPIO_ReadInputDataBit(TRAIL_SENSOR_PORT, TRAIL_SENSOR_LEFT);
    uint8_t IL = GPIO_ReadInputDataBit(TRAIL_SENSOR_PORT, TRAIL_SENSOR_LEFT_MID);
    uint8_t IR = GPIO_ReadInputDataBit(TRAIL_SENSOR_PORT, TRAIL_SENSOR_RIGHT_MID);
    uint8_t R = GPIO_ReadInputDataBit(TRAIL_SENSOR_PORT, TRAIL_SENSOR_RIGHT);
    
    // 在OLED第3行显示传感器状态用于调试
    OLED_ShowString(3, 1, "Sensors:        ");
    OLED_ShowNum(3, 10, L, 1);
    OLED_ShowNum(3, 12, IL, 1);
    OLED_ShowNum(3, 14, IR, 1);
    OLED_ShowNum(3, 16, R, 1);
    
    // 1. 特殊状态处理
    if (L == 1 && IL == 1 && IR == 1 && R == 1) {
        // 全部检测到：保持直行
        Car_Forward(TRAIL_SPEED);
        OLED_ShowString(4, 1, "Trail:Cross Road");
        return;
    }
    
    if (L == 0 && IL == 0 && IR == 0 && R == 0) {

//		Car_SpinLeft(TRAIL_TURN_SPEED);
//        Delay_ms(200);
//		Car_Stop();
//		Delay_ms(200);
//		if(L == 0 && IL == 0 && IR == 0 && R == 0)
//		{
//			Car_SpinRight(TRAIL_TURN_SPEED);
//			Delay_ms(200);
//			Car_Stop();
//			Delay_ms(200);
//			if(!(L == 0 && IL == 0 && IR == 0 && R == 0))
//			{
//				return;
//			}
//		}
		
		Car_Forward(TRAIL_SPEED);
		
        OLED_ShowString(4, 1, "Trail:Lost Line ");
		return;
    }
    
    // 2. 新增直行条件：当左右都为0时，中间两个只要有一个为1，小车就直行
    if (L == 0 && R == 0 && (IL == 1 || IR == 1)) {       
        Car_Forward(TRAIL_SPEED);
        OLED_ShowString(4, 1, "Trail:Forward   ");
        return;
    }
    
    // 3. 三个模块检测到赛道
    // L、IL、IR检测到（缺R）：轻微左转
    if (L == 1 && IL == 1 && IR == 1 && R == 0) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinLeft(TRAIL_TURN_SPEED);
        Delay_ms(200);
        OLED_ShowString(4, 1, "Trail:Soft Left ");
        return;
    }
    // IL、IR、R检测到（缺L）：轻微右转
    else if (L == 0 && IL == 1 && IR == 1 && R == 1) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinRight(TRAIL_TURN_SPEED);
        Delay_ms(200);
        OLED_ShowString(4, 1, "Trail:Soft Right");
        return;
    }
    // L、IL、R检测到（缺IR）：直行
    else if (L == 1 && IL == 1 && IR == 0 && R == 1) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_Forward(TRAIL_SPEED);
        Delay_ms(500);
        OLED_ShowString(4, 1, "Trail:Forward   ");
        return;
    }
    // L、IR、R检测到（缺IL）：直行
    else if (L == 1 && IL == 0 && IR == 1 && R == 1) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_Forward(TRAIL_SPEED);
        Delay_ms(500);
        OLED_ShowString(4, 1, "Trail:Forward   ");
        return;
    }
    
    // 4. 两个模块检测到赛道
    // L和IL同时检测到：轻微左转
    else if (L == 1 && IL == 1 && IR == 0 && R == 0) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinLeft(TRAIL_TURN_SPEED);
        Delay_ms(100);
        OLED_ShowString(4, 1, "Trail:Soft Left ");
        return;
    }
    // IL和IR同时检测到：直行
    else if (L == 0 && IL == 1 && IR == 1 && R == 0) {
        Car_Forward(TRAIL_SPEED);
        OLED_ShowString(4, 1, "Trail:Forward   ");
        return;
    }
    // IR和R同时检测到：轻微右转
    else if (L == 0 && IL == 0 && IR == 1 && R == 1) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinRight(TRAIL_TURN_SPEED);
        Delay_ms(100);
        OLED_ShowString(4, 1, "Trail:Soft Right");
        return;
    }
    // L和IR同时检测到：中等左转（异常情况处理）
    else if (L == 1 && IL == 0 && IR == 1 && R == 0) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinLeft(TRAIL_TURN_SPEED);
        Delay_ms(200);
        OLED_ShowString(4, 1, "Trail:Mid Left  ");
        return;
    }
    // IL和R同时检测到：中等右转（异常情况处理）
    else if (L == 0 && IL == 1 && IR == 0 && R == 1) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinRight(TRAIL_TURN_SPEED);
        Delay_ms(200);
        OLED_ShowString(4, 1, "Trail:Mid Right ");
        return;
    }
    
    // 5. 单个模块检测到赛道
    // 只有L检测到：强力左转
    else if(L == 1 && IL == 0 && IR == 0 && R == 0) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinLeft(TRAIL_TURN_SPEED);
        Delay_ms(300);
        OLED_ShowString(4, 1, "Trail:Hard Left ");
    }
    // 只有R检测到：强力右转
    else if(L == 0 && IL == 0 && IR == 0 && R == 1) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinRight(TRAIL_TURN_SPEED);
        Delay_ms(300);
        OLED_ShowString(4, 1, "Trail:Hard Right");
    }
    // 只有IL检测到：中等左转
    else if(L == 0 && IL == 1 && IR == 0 && R == 0) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinLeft(TRAIL_TURN_SPEED);
        Delay_ms(200);
        OLED_ShowString(4, 1, "Trail:Mid Left  ");
    }
    // 只有IR检测到：中等右转
    else if(L == 0 && IL == 0 && IR == 1 && R == 0) {
//        Car_Stop();
//        Delay_ms(1000);
        Car_SpinRight(TRAIL_TURN_SPEED);
        Delay_ms(200);
        OLED_ShowString(4, 1, "Trail:Mid Right ");
    }
    // 默认情况
    else {
        Car_Stop();
        OLED_ShowString(4, 1, "Trail:Stop      ");
    }
}

void Trail_Start(void)
{
    trail_enabled = 1;
    Car_Stop();  // 先停止，等待传感器检测
}

void Trail_Stop(void)
{
    trail_enabled = 0;
    Car_Stop();
    OLED_ShowString(4, 1, "                ");
}

uint8_t Trail_IsRunning(void)
{
    return trail_enabled;
}
