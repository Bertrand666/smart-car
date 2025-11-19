#ifndef __CAR_CONTROL_H
#define __CAR_CONTROL_H

void Car_Forward(uint16_t speed);
void Car_Backward(uint16_t speed);
void Car_TurnLeft(uint16_t speed);
void Car_TurnRight(uint16_t speed);
void Car_SpinLeft(uint16_t speed);
void Car_SpinRight(uint16_t speed);
void Car_Stop(void);
void Car_Brake(void);
void Car_Accelerate(uint16_t target_speed, uint16_t steps);

#endif
