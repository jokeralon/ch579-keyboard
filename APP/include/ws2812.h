#ifndef __WS2812_H
#define __WS2812_H

#include "CONFIG.h"
#include "CH57x_common.h"

#define WS2812_NUM		63
#define RGB_NULL			0xff
#define COLOR_MAX		0XDD

#define WS2812_EVENT_START		1

#define GPIO_PortA		0
#define GPIO_PortB		1

#define WS2812_PIN			GPIO_Pin_14
#define WS2812_PORT			GPIO_PortB

void RGB_GPIO_Init(void);
void WS2812_Init(void);

void RGB_24Bit(uint32 num, uint8 r, uint8 g, uint8 b);
#endif




