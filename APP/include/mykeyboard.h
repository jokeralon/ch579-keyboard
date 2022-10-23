#ifndef __MYKEYBOARD_H
#define __MYKEYBOARD_H

#include "CONFIG.h"
#include "CH57x_common.h"

#define BLE_MODE		1
#define USB_MODE		!BLE_MODE

#define KEYBOARD_SCANF_EVENT	0xee

#define GPIO_PortA		0
#define GPIO_PortB		1

#define ROW_NUM				5
#define COL_NUM				14

#define GPIO_HIGH			1
#define GPIO_LOW			0
#define GPIO_NONE			2

#define EC11_SW				0xff
#define MODE_SW				0xf2

#define KEY_BOOT_PORT	GPIO_PortB
#define KEY_BOOT_PIN	GPIO_Pin_22

#define ROW1_PIN			GPIO_Pin_7
#define ROW1_PORT			GPIO_PortB
#define ROW2_PIN			GPIO_Pin_6
#define ROW2_PORT			GPIO_PortB
#define ROW3_PIN			GPIO_Pin_5
#define ROW3_PORT			GPIO_PortB
#define ROW4_PIN			GPIO_Pin_4
#define ROW4_PORT			GPIO_PortB
#define ROW5_PIN			GPIO_Pin_3
#define ROW5_PORT			GPIO_PortB

#define COL1_PIN			GPIO_Pin_5
#define COL1_PORT			GPIO_PortA
#define COL2_PIN			GPIO_Pin_6
#define COL2_PORT			GPIO_PortA
#define COL3_PIN			GPIO_Pin_0
#define COL3_PORT			GPIO_PortA
#define COL4_PIN			GPIO_Pin_1
#define COL4_PORT			GPIO_PortA
#define COL5_PIN			GPIO_Pin_2
#define COL5_PORT			GPIO_PortA
#define COL6_PIN			GPIO_Pin_3
#define COL6_PORT			GPIO_PortA
#define COL7_PIN			GPIO_Pin_15
#define COL7_PORT			GPIO_PortA
#define COL8_PIN			GPIO_Pin_14
#define COL8_PORT			GPIO_PortB
#define COL9_PIN			GPIO_Pin_13
#define COL9_PORT			GPIO_PortA
#define COL10_PIN			GPIO_Pin_2
#define COL10_PORT		    GPIO_PortB
#define COL11_PIN			GPIO_Pin_1
#define COL11_PORT		    GPIO_PortB
#define COL12_PIN			GPIO_Pin_0
#define COL12_PORT		    GPIO_PortB
#define COL13_PIN			GPIO_Pin_21
#define COL13_PORT		    GPIO_PortB
#define COL14_PIN			GPIO_Pin_20
#define COL14_PORT		    GPIO_PortB

typedef struct mykeyboard
{
    uint8 Boot_key;
    uint8 FN_key;
    uint8 RAlt_key;
    uint8 LAlt_key;
    uint8 RCtrl_key;
    uint8 LCtrl_key;
    uint8 RShift_key;
    uint8 LShift_key;
    uint8 ESC_key;
}keyboard_func_key_status_t;


void Keyboard_Init(void);

void keyBoard_GPIO_Init(void);
void keyBoard_Scanf( void );
void keyBoard_GPIO_SetPin( uint8 port, uint32 pin, uint8 status);
uint8 keyBoard_GPIO_ReadPin( uint8 port, uint32 pin);
uint8 keyBoard_SendValue( void );
#endif
