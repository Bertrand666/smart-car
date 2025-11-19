#ifndef __SPP_SERIAL_H
#define __SPP_SERIAL_H

#include <stdio.h>
#include <stdarg.h>
#include "OLED.h"
#include "car_control.h"
#include "string.h"
#include "Delay.h"

extern char Serial_RxPacket[];
extern uint8_t Serial_RxFlag;

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);

void spp_control(void);

#endif
