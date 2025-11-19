#include "stm32f10x.h"
#include "HCSR04.h"
#include "Delay.h"
#include "OLED.h"

// 全局变量
static volatile uint8_t capture_flag = 0;      // 捕获状态标志：0-未捕获，1-已捕获上升沿，2-已捕获下降沿
static volatile uint32_t capture_start = 0;    // 上升沿捕获时间戳
static volatile uint32_t capture_end = 0;      // 下降沿捕获时间戳
static volatile uint32_t distance_cm = 0;      // 计算得到的距离值，单位：cm

void HCSR04_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    // 初始化Trig引脚 (PA1) - 推挽输出
    GPIO_InitStructure.GPIO_Pin = ULTRASONIC_TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ULTRASONIC_TRIG_PORT, &GPIO_InitStructure);
    
    // 初始化Echo引脚 (PA11) - 浮空输入，TIM1通道4
    GPIO_InitStructure.GPIO_Pin = ULTRASONIC_ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ULTRASONIC_ECHO_PORT, &GPIO_InitStructure);
    
    // Trig初始低电平
    GPIO_ResetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);
    
    // TIM1时基初始化 - 1MHz频率
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    // 输入捕获配置 - 通道4 (PA11)
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x00;
    TIM_ICInit(TIM1, &TIM_ICInitStructure);
    
    // 使能捕获中断
    TIM_ITConfig(TIM1, TIM_IT_CC4, ENABLE);
    
    // NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM1, ENABLE);
    
    capture_flag = 0;
}

// 获取距离测量结果（单位：cm）- 带滑动窗口滤波的稳定测量
uint32_t HCSR04_GetDistance(void)
{
    static uint32_t history[5] = {0, 0, 0, 0, 0};  // 历史数据滑动窗口，用于滤波
    static uint8_t history_index = 0;               // 历史数据索引，指向当前写入位置
    static uint8_t history_filled = 0;              // 历史数据是否已填满标志
    
    uint32_t pulse_width, measured_distance = 0;
    
    // 发送触发脉冲
    GPIO_SetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);
    Delay_us(10);  // 标准10us触发脉冲
    GPIO_ResetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);
    
    // 重置捕获状态
    capture_flag = 0;
    capture_start = 0;
    capture_end = 0;
    
    // 等待测量完成（带超时）
    uint32_t timeout = 0;
    while(capture_flag != 2 && timeout < ULTRASONIC_TIMEOUT) {
        timeout++;
        Delay_us(1);
    }
    
    if(capture_flag == 2 && capture_start != 0 && capture_end != 0) {
        // 计算脉冲宽度
        if(capture_end >= capture_start) {
            pulse_width = capture_end - capture_start;
        } else {
            pulse_width = (0xFFFF - capture_start) + capture_end;
        }
        
        // 计算距离
        measured_distance = (pulse_width * 100) / 5882;
        
        // 限制有效范围（2cm - 400cm）
        if(measured_distance < 2) measured_distance = 0;
        if(measured_distance > 400) measured_distance = 0;
        
        // 如果测量有效，更新历史数据
        if (measured_distance > 0) {
            history[history_index] = measured_distance;
            history_index = (history_index + 1) % 5;
            if (!history_filled && history_index == 0) {
                history_filled = 1;
            }
            
            // 如果历史数据已填充，使用滑动窗口滤波
            if (history_filled) {
                // 计算历史数据的平均值
                uint32_t sum = 0;
                for (int i = 0; i < 5; i++) {
                    sum += history[i];
                }
                measured_distance = sum / 5;
            }
        }
    }
    
    distance_cm = measured_distance;
    return measured_distance;
}

// 显示距离函数
void HCSR04_DisplayDistance(void)
{
    uint32_t distance = HCSR04_GetDistance();
    OLED_ShowString(2, 1, "Dist:           ");	
    if(distance == 0) {
        OLED_ShowString(2, 1, "No Signal       ");
    } else {
        OLED_ShowNum(2, 7, distance, 3);
		OLED_ShowString(2, 11, "cm");
    }
}

// TIM1捕获比较中断处理函数
void TIM1_CC_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM1, TIM_IT_CC4) != RESET) {
        if(capture_flag == 0) {
            // 第一次捕获 - 上升沿
            capture_start = TIM_GetCapture4(TIM1);
            TIM_OC4PolarityConfig(TIM1, TIM_ICPolarity_Falling);
            capture_flag = 1;
        } else if(capture_flag == 1) {
            // 第二次捕获 - 下降沿
            capture_end = TIM_GetCapture4(TIM1);
            TIM_OC4PolarityConfig(TIM1, TIM_ICPolarity_Rising);
            capture_flag = 2;
        }
        
        TIM_ClearITPendingBit(TIM1, TIM_IT_CC4);
    }
}
