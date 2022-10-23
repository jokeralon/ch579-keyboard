#ifndef __WS2812_H
#define __WS2812_H

#include "CONFIG.h"
#include "CH57x_common.h"

#if defined( __CC_ARM )
#pragma anon_unions
#endif

typedef union
{
    struct
    {
        uint8 b;
        uint8 g;
        uint8 r;
    };
    uint32 rgb;
} ws2812_rgb_t;

#define WS2812_NUM 63
#define RGB_NULL 0xff

#define COLOR_FADE_STEP     (7)
#define COLOR_RED_MAX 0xDD
#define COLOR_GREEN_MAX 0x00
#define COLOR_BLUE_MAX 0x00

#define WS2812_EVENT_START 1

#define GPIO_PortA 0
#define GPIO_PortB 1

#define WS2812_PIN GPIO_Pin_14
#define WS2812_PORT GPIO_PortB

void RGB_GPIO_Init(void);
void WS2812_Init(void);

void RGB_24Bit(uint32 num, uint8 r, uint8 g, uint8 b);
#endif
