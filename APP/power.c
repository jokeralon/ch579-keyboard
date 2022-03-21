#include "power.h"

/* ��Դ��������ID */ 
uint8_t PowerManage_TaskId;
/* ���ĵȼ� */ 
uint8_t LowPowerLevel = 0;
/* ��Դ�����¼����� */ 
uint16_t PowerManage_ProcessEvent( uint8_t task_id, uint16_t events );


/*******************************************************************************
 * @fn      PowerManage_Init
 *
 * @brief   ��Դ��������ʼ��
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void PowerManage_Init(void)
{
	/* ע������ */ 
	PowerManage_TaskId = TMOS_ProcessEventRegister(PowerManage_ProcessEvent);
	
	/* �������� */ 
	GPIOB_ModeCfg( GPIO_Pin_1, GPIO_ModeIN_PU ); 
	GPIOB_ITModeCfg( GPIO_Pin_1, GPIO_ITMode_FallEdge );        // �½��ػ���
	/* ��GPIO�ж� */ 
	NVIC_EnableIRQ( GPIO_IRQn );
	/* ����ΪGPIO���� */ 
  PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_GPIO_WAKE );
	/* ��ʼ ��Դ�������� ���� */ 
	tmos_start_task( PowerManage_TaskId , LOW_POWER_SERVICE ,1600 );
}

/*******************************************************************************
 * @fn      PowerManage_ProcessEvent
 *
 * @brief   ��Դ����������
 *
 * @param   task_id : ����ID
 *          events  : �¼�
 *
 * @return  tmosEvents
 *******************************************************************************/
tmosEvents PowerManage_ProcessEvent( tmosTaskID task_id, tmosEvents events )
{
	uint8 * msgPtr;

  if( events & SYS_EVENT_MSG ){  // ����HAL����Ϣ������tmos_msg_receive��ȡ��Ϣ��������ɺ�ɾ����Ϣ��
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
 * @brief   �͹��ķ�����
 *
 * @param   void
 *
 * @return  none
 *******************************************************************************/
void LowPowerSerive(void)
{
	switch( LowPowerLevel )
	{
		case 0:/* �������� */ 
			/* ��USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL |= (0x03 << 4);
		
			break;
		case 1:/* �������� */ 
			/* �ر�USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL &= ~(0x03 << 4);
		
			PRINT( "IDLE mode sleep \n");   
			DelayMs(1);
			LowPower_Idle();
			PRINT( "wake.. \n"); 
			DelayMs(2000);   
			break;
		case 2:
			/* �ر�USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL &= ~(0x03 << 4);		
		
			PRINT( "Halt_1 mode sleep \n");   
			DelayMs(1);
			LowPower_Halt_1();
/*
ʹ��HSI/5=6.4M˯�ߣ�����ʱ������Ҫ 2048Tsys��330us
HSE����һ�㲻����1.2ms(500us-1200us)�������л����ⲿHSE����Ҫ 1.2ms-330us ���ʱ����Ա�֤HSE�㹻�ȶ���һ����������
DelayUs()����ʱ����32Mʱ�ӵ���д����ʱ��ƵΪ6.4M������ DelayUs((1200-330)/5)    
*/
			if(!(R8_HFCK_PWR_CTRL&RB_CLK_XT32M_PON)) {     // �Ƿ�HSE�ϵ�
					PWR_UnitModCfg( ENABLE, UNIT_SYS_HSE );   // HSE�ϵ�
					DelayUs((1200)/5);
			}
			else if(!(R16_CLK_SYS_CFG&RB_CLK_OSC32M_XT)){   // �Ƿ�ѡ�� HSI/5 ��ʱ��Դ
					DelayUs((1200-330)/5);
			} 
			HSECFG_Current( HSE_RCur_100 );     // ��Ϊ�����(�͹��ĺ�����������HSEƫ�õ���)
			DelayUs(5/5);                       // �ȴ��ȶ� 1-5us
			SetSysClock( CLK_SOURCE_HSE_32MHz );
			PRINT( "wake.. \n"); 
			DelayMs(2000);    
			break;
		case 3:
			/* �ر�USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL &= ~(0x03 << 4);
		
  		PRINT( "Halt_2 mode sleep \n");   
			DelayMs(1);
			LowPower_Halt_2();
/*
ʹ��HSI/5=6.4M˯�ߣ�����ʱ������Ҫ 2048Tsys��330us
HSE����һ�㲻����1.2ms(500us-1200us)�������л����ⲿHSE����Ҫ 1.2ms-330us ���ʱ����Ա�֤HSE�㹻�ȶ���һ����������
DelayUs()����ʱ����32Mʱ�ӵ���д����ʱ��ƵΪ6.4M������ DelayUs((1200-330)/5)    
*/
			if(!(R8_HFCK_PWR_CTRL&RB_CLK_XT32M_PON)) {     // �Ƿ�HSE�ϵ�
					PWR_UnitModCfg( ENABLE, UNIT_SYS_HSE );   // HSE�ϵ�
					DelayUs((1200)/5);
			}
			else if(!(R16_CLK_SYS_CFG&RB_CLK_OSC32M_XT)){   // �Ƿ�ѡ�� HSI/5 ��ʱ��Դ
					DelayUs((1200-330)/5);
			} 
			HSECFG_Current( HSE_RCur_100 );     // ��Ϊ�����(�͹��ĺ�����������HSEƫ�õ���)
			DelayUs(5/5);                       // �ȴ��ȶ� 1-5us
			SetSysClock( CLK_SOURCE_HSE_32MHz );
			PRINT( "wake.. \n"); 
			DelayMs(2000); 
			break;
		case 4:
			/* �ر�USB */ 
			R8_USB_CTRL &= ~RB_UC_HOST_MODE;
			R8_USB_CTRL &= ~(0x03 << 4);
		
			PRINT( "shut down mode sleep \n");   
			DelayMs(1);
			LowPower_Shutdown( NULL );                          //ȫ���ϵ磬���Ѻ�λ
/* 
   ��ģʽ���Ѻ��ִ�и�λ������������벻�����У�
   ע��Ҫȷ��ϵͳ˯��ȥ�ٻ��Ѳ��ǻ��Ѹ�λ�������п��ܱ��IDLE�ȼ����� 
*/
/*
ʹ��HSI/5=6.4M˯�ߣ�����ʱ������Ҫ 2048Tsys��330us
HSE����һ�㲻����1.2ms(500us-1200us)�������л����ⲿHSE����Ҫ 1.2ms-330us ���ʱ����Ա�֤HSE�㹻�ȶ���һ����������
DelayUs()����ʱ����32Mʱ�ӵ���д����ʱ��ƵΪ6.4M������ DelayUs((1200-330)/5)    
*/
			if(!(R8_HFCK_PWR_CTRL&RB_CLK_XT32M_PON)) {     // �Ƿ�HSE�ϵ�
					PWR_UnitModCfg( ENABLE, UNIT_SYS_HSE );   // HSE�ϵ�
					DelayUs((1200)/5);
			}
			else if(!(R16_CLK_SYS_CFG&RB_CLK_OSC32M_XT)){   // �Ƿ�ѡ�� HSI/5 ��ʱ��Դ
					DelayUs((1200-330)/5);
			} 
			HSECFG_Current( HSE_RCur_100 );     // ��Ϊ�����(�͹��ĺ�����������HSEƫ�õ���)
			DelayUs(5/5);                       // �ȴ��ȶ� 1-5us
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
 * @brief   GPIO�ж� �����л��͹���ģʽ
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

