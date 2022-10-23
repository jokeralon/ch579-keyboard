/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 蓝牙键盘应用主函数及任务系统初始化
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "CH57x_common.h"
#include "HAL.h"
#include "hiddev.h"
#include "hidkbd.h"
#include "power.h"
#include "mykeyboard.h"
#include "ws2812.h"

/* ws2812.h */
extern uint8_t rgb_r, rgb_g, rgb_b;

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

// 蓝牙数据传输
// void hidEmuSendKbdReport( uint8 keycode );
__align(4) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if (defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

extern uint8_t LowPowerLevel;

/*******************************************************************************
 * @fn      TMOS_InitTasks
 *
 * @brief   This function invokes the initialization function for each task.
 *
 * @param   void
 *
 * @return  none
 *******************************************************************************/
void TMOS_InitTasks(void)
{
	/* CH57X BLE Init */
	CH57X_BLEInit();
	PRINT("CH57X_BLEInit\r\n");
	/* HAL Init */
	HAL_Init();
	PRINT("HAL_Init\r\n");
	/* BLE GAPRole Init */
	GAPRole_PeripheralInit();
	PRINT("GAPRole_PeripheralInit\r\n");
	/* WS2812 Init */
	WS2812_Init();
	PRINT("WS2812_Init\r\n");
	/* HidDev Init */
	HidDev_Init();
	PRINT("HidDev_Init\r\n");
	/* HidEmu Init */
	HidEmu_Init();
	PRINT("HidEmu_Init\r\n");
	/* USB HID Device Init */
	UsbHid_Init();
	PRINT("UsbHid_Init\r\n");
	/* Keyboard Init */
	Keyboard_Init();
	PRINT("keyBoard_Init\r\n");
	//	/* Power Manage Init */
	//	PowerManage_Init();
	//	PRINT("PowerManage_Init\r\n");

	DelayMs(500);
}

/*******************************************************************************
 * Function Name  : main
 * Description    : 主函数
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
	//  GPIOA_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PU );
	//  GPIOB_ModeCfg( GPIO_Pin_All&(~(GPIO_Pin_11|GPIO_Pin_10)), GPIO_ModeIN_PU );
	//
	//	GPIOB_ModeCfg(GPIO_Pin_18 | GPIO_Pin_19, GPIO_ModeOut_PP_5mA );

	/* 外部32M时钟 */
	SetSysClock(CLK_SOURCE_HSE_32MHz);

	HClk32M_Select(Clk32M_HSE);
	/* 打开PLL */
	PWR_UnitModCfg(ENABLE, UNIT_SYS_PLL);

	/* 低功耗处理，IO全部拉高 */
	//	  GPIOA_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PD );
	//    GPIOB_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PD );
	DelayMs(5);

#ifdef DEBUG
	GPIOA_SetBits(bTXD1);
	GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
	UART1_DefInit();
#endif
	PRINT("%s\n", VER_LIB);
	TMOS_InitTasks();

	while (1)
	{
		TMOS_SystemProcess();
	}
}

/******************************** endfile @ main ******************************/
