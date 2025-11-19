#include "HCSR04_Follow.h"
#include "OLED.h"

// 跟随模式状态变量
static uint8_t follow_enabled = 0;        // 跟随模式使能标志：0-禁用，1-启用
static uint32_t last_distance = 0;        // 上一次测量的距离值，单位：cm
static uint8_t last_state = 0;            // 上一次的状态：0-停止，1-前进
static uint8_t stable_count = 0;          // 稳定计数，防止状态切换抖动

void HCSR04_Follow_Init(void)
{
    follow_enabled = 0;
    last_distance = 0;
    last_state = 0;
    stable_count = 0;
}

void HCSR04_Follow_Update(uint32_t distance)
{
    static uint8_t zero_count = 0;  // 零距离计数，用于防抖
    
    if (!follow_enabled) return;
    
    // 处理距离为0的情况 - 添加防抖机制
    if (distance == 0) {
        // 只有当连续3次测量到0距离时才停止小车
        // 这样可以避免因测量波动导致的间歇停止
        zero_count++;
        
        if (zero_count >= 3) {
            Car_Stop();
            stable_count = 0;
            OLED_ShowString(4, 1, "No Signal ");
        }
    } 
    else {
        // 重置零距离计数
        zero_count = 0;
        
        // 控制逻辑 - 添加防抖机制
        // 修改为：距离<10cm前进，>10cm停止
        uint8_t current_state = (distance < FOLLOW_DISTANCE_THRESHOLD) ? 1 : 0;
        
        // 防抖逻辑：连续3次相同状态才切换
        if (current_state == last_state) {
            stable_count++;
            if (stable_count >= 3) {
                // 稳定状态，执行控制
                if (current_state == 1) {
                    Car_Forward(FOLLOW_SPEED);
                    OLED_ShowString(4, 1, "Following");
                } else {
                    Car_Stop();
                    OLED_ShowString(4, 1 , "Stop     ");
                }
            }
        } else {
            // 状态变化，重置稳定计数
            stable_count = 0;
            last_state = current_state;
        }
    }
    
    last_distance = distance;
}

void HCSR04_Follow_Start(void)
{
    follow_enabled = 1;
    // 重置状态变量，确保第一次更新时能够正确判断
    last_distance = 0;
    last_state = 0;
}

void HCSR04_Follow_Stop(void)
{
    follow_enabled = 0;
    Car_Stop();
    OLED_ShowString(4, 1, "                ");
}

uint8_t HCSR04_Follow_IsRunning(void)
{
    return follow_enabled;
}
