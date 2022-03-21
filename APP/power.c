#include "power.h"

/* 电源管理任务ID */ 
uint8_t PowerManage_TaskId;
/* 功耗等级 */ 
uint8_t LowPowerLevel = 0;
/* 电源管理事件处理 */ 
uint16_t PowerManage_ProcessEvent( uint8_t task_id, uint16_t events );


/*******************************************************************************
 * @fn      PowerManage_Init
 *
 * @brief   电源管理服务初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void PowerManage_Init(void)
{
	/* 注册任务 */ 
	PowerManage_TaskId = TMOS_ProcessEventRegister(PowerManage_ProcessEvent);
	
	/* 按键设置 */ 
	GPIOB_ModeCfg( GPIO_Pin_1, GPIO_ModeIN_PU ); 
	GPIOB_ITModeCfg( GPIO_Pin_1, GPIO_ITMode_FallEdge );        // 下降沿唤醒
	/* 开GPIO中断 */ 
	NVIC_EnableIRQ( GPIO_IRQn );
	/* 设置为GPIO唤醒 */ 
  PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_GPIO_WAKE );
	/* 开始 电源管理任务 任务 */ 
	tmos_start_task( PowerManage_TaskId , LOW_POWER_SERVICE ,1600 );
}

/*******************************************************************************
 * @fn      PowerManage_ProcessEvent
 *
 * @brief   电源管理任务处理
 *
 * @param   task_id : 任务ID
 *          events  : 事件
 *
 * @return  tmosEvents
 *******************************************************************************/
tmosEvents PowerManage_ProcessEvent( tmosTaskID task_id, tmosEvents events )
{
	uint8 * msgPtr;

  if( events & SYS_EVENT_MSG ){  // 处理HAL层消息，调用tmos_msg_receive读取消息，处理完成后删除消息。
		msgPtr = tmos_msg_receive(task_id);
    if( msgPtr ){
      /* De-allocate */
      tmos_msg_deallocate( msgPtr );
    }
    return events ^ SYS_EVENT_MSG;
	}
  if( events & LOW_POWER_SERVICE ){
		LowPowerSerive();
    tmos_start_task( PowerManage_TaskId , LOW_POWER_SERVICE ,800 ); //  625us / run  1000ms
		return events ^ LOW_POWER_SERVICE;
  }
  return 0;
}

/*******************************************************************************
 * @fn      LowProwerSerive
 *
 * @brief   低功耗服务函数
 *
 * @param   void
 *
 * @return  none
 *******************************************************************************/
void LowPowerSerive(void)
{
	switch( LowPowerLevel )
	{
		case 0:/* 正常运行 */ 
			/* 打开USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL |= (0x03 << 4);
		
			break;
		case 1:/* 空闲运行 */ 
			/* 关闭USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL &= ~(0x03 << 4);
		
			PRINT( "IDLE mode sleep \n");   
			DelayMs(1);
			LowPower_Idle();
			PRINT( "wake.. \n"); 
			DelayMs(2000);   
			break;
		case 2:
			/* 关闭USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL &= ~(0x03 << 4);		
		
			PRINT( "Halt_1 mode sleep \n");   
			DelayMs(1);
			LowPower_Halt_1();
/*
使用HSI/5=6.4M睡眠，唤醒时间大概需要 2048Tsys≈330us
HSE起振一般不超过1.2ms(500us-1200us)，所以切换到外部HSE，需要 1.2ms-330us 这个时间可以保证HSE足够稳定，一般用于蓝牙
DelayUs()函数时基于32M时钟的书写，此时主频为6.4M，所以 DelayUs((1200-330)/5)    
*/
			if(!(R8_HFCK_PWR_CTRL&RB_CLK_XT32M_PON)) {     // 是否HSE上电
					PWR_UnitModCfg( ENABLE, UNIT_SYS_HSE );   // HSE上电
					DelayUs((1200)/5);
			}
			else if(!(R16_CLK_SYS_CFG&RB_CLK_OSC32M_XT)){   // 是否选择 HSI/5 做时钟源
					DelayUs((1200-330)/5);
			} 
			HSECFG_Current( HSE_RCur_100 );     // 降为额定电流(低功耗函数中提升了HSE偏置电流)
			DelayUs(5/5);                       // 等待稳定 1-5us
			SetSysClock( CLK_SOURCE_HSE_32MHz );
			PRINT( "wake.. \n"); 
			DelayMs(2000);    
			break;
		case 3:
			/* 关闭USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL &= ~(0x03 << 4);
		
  		PRINT( "Halt_2 mode sleep \n");   
			DelayMs(1);
			LowPower_Halt_2();
/*
使用HSI/5=6.4M睡眠，唤醒时间大概需要 2048Tsys≈330us
HSE起振一般不超过1.2ms(500us-1200us)，所以切换到外部HSE，需要 1.2ms-330us 这个时间可以保证HSE足够稳定，一般用于蓝牙
DelayUs()函数时基于32M时钟的书写，此时主频为6.4M，所以 DelayUs((1200-330)/5)    
*/
			if(!(R8_HFCK_PWR_CTRL&RB_CLK_XT32M_PON)) {     // 是否HSE上电
					PWR_UnitModCfg( ENABLE, UNIT_SYS_HSE );   // HSE上电
					DelayUs((1200)/5);
			}
			else if(!(R16_CLK_SYS_CFG&RB_CLK_OSC32M_XT)){   // 是否选择 HSI/5 做时钟源
					DelayUs((1200-330)/5);
			} 
			HSECFG_Current( HSE_RCur_100 );     // 降为额定电流(低功耗函数中提升了HSE偏置电流)
			DelayUs(5/5);                       // 等待稳定 1-5us
			SetSysClock( CLK_SOURCE_HSE_32MHz );
			PRINT( "wake.. \n"); 
			DelayMs(2000); 
			break;
		case 4:
			/* 关闭USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL &= ~(0x03 << 4);
		
			PRINT( "shut down mode sleep \n");   
			DelayMs(1);
			LowPower_Shutdown( NULL );                          //全部断电，唤醒后复位
/* 
   此模式唤醒后会执行复位，所以下面代码不会运行，
   注意要确保系统睡下去再唤醒才是唤醒复位，否则有可能变成IDLE等级唤醒 
*/
/*
使用HSI/5=6.4M睡眠，唤醒时间大概需要 2048Tsys≈330us
HSE起振一般不超过1.2ms(500us-1200us)，所以切换到外部HSE，需要 1.2ms-330us 这个时间可以保证HSE足够稳定，一般用于蓝牙
DelayUs()函数时基于32M时钟的书写，此时主频为6.4M，所以 DelayUs((1200-330)/5)    
*/
			if(!(R8_HFCK_PWR_CTRL&RB_CLK_XT32M_PON)) {     // 是否HSE上电
					PWR_UnitModCfg( ENABLE, UNIT_SYS_HSE );   // HSE上电
					DelayUs((1200)/5);
			}
			else if(!(R16_CLK_SYS_CFG&RB_CLK_OSC32M_XT)){   // 是否选择 HSI/5 做时钟源
					DelayUs((1200-330)/5);
			} 
			HSECFG_Current( HSE_RCur_100 );     // 降为额定电流(低功耗函数中提升了HSE偏置电流)
			DelayUs(5/5);                       // 等待稳定 1-5us
			SetSysClock( CLK_SOURCE_HSE_32MHz );
			PRINT( "wake.. \n");
			DelayMs(500);
			break;
		default:break;
	}
}



/*******************************************************************************
 * @fn      GPIO_IRQHandler
 *
 * @brief   GPIO中断 用于切换低功耗模式
 *
 * @param   void
 *
 * @return  none
 *******************************************************************************/
void GPIO_IRQHandler(void)
{
  GPIOB_ClearITFlagBit( GPIO_Pin_1 );
	LowPowerLevel++;
	if(LowPowerLevel>=5 )
		LowPowerLevel = 0;
}

