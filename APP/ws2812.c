#include "ws2812.h"
#include "mykeyboard.h"


extern uint8 keyboard_mode_flag;									// ģʽ��־
extern uint8 keyboard_mode_rgb_flag;							// ģʽRGB��־
extern uint8 Keyboard_Map_Temp[ROW_NUM][COL_NUM];	// �����
uint8_t rgb_record_temp[ROW_NUM][COL_NUM];				// ʵʱ������

uint8_t WS2812_Taskid = 0;												// WS2812 �¼�ID
unsigned char CODE1 = 0xFC;
unsigned char CODE0 = 0xC0;

void RGB_LED_Reset(void);
void RGB_LED_Write_24Bits(uint8 g, uint8 r, uint8 b);
tmosEvents WS2812_ProcessEvent( tmosTaskID task_id, tmosEvents events );

unsigned char reset[3] = {0,0,0};									// ��λ����
uint8_t rgb_buf[3*WS2812_NUM]={0x00};							// RGB ����


/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 ���� GPIO��ʼ��
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void RGB_GPIO_Init(void)
{
    GPIOA_ModeCfg(GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    SPI0_MasterDefInit( );									// SPI0 ��ʼ�� 4��Ƶ
	
		RGB_LED_Write_24Bits(0X00,0X00,0X00);		
	
		// WS2812 ��λ	
		R8_SPI0_CTRL_MOD &= !RB_SPI_MOSI_OE;		// SPI0 MOSI ��ֹ
		R8_SPI0_CTRL_MOD &= !RB_SPI_SCK_OE;			// SPI0 SCK ��ֹ
		GPIOA_ModeCfg( GPIO_Pin_14, GPIO_ModeOut_PP_5mA );
		GPIOA_ResetBits( GPIO_Pin_14 );
	
		mDelayuS(80);
		R8_SPI0_CTRL_MOD |= RB_SPI_MOSI_OE;
		RGB_LED_Write_24Bits(0X00,0X00,0X00);
		RGB_LED_Reset();
	
}
/*******************************************************************************
 * @fn      RGB_LED_Reset
 *
 * @brief   WS2812 �������
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void RGB_LED_Reset(void)
{
//	RGB_LED_LOW;
	
	R8_SPI0_CTRL_MOD &= !RB_SPI_MOSI_OE; //MOSI ��ֹ
	mDelayuS(1);
	GPIOA_ModeCfg( GPIO_Pin_14, GPIO_ModeOut_PP_5mA );
	GPIOA_ResetBits( GPIO_Pin_14 );
	
	mDelayuS(80);
}
/*******************************************************************************
 * @fn      RGB_LED_Write_24Bits
 *
* @brief   WS2812 ����һ���ƹ����� 24bit
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void RGB_LED_Write_24Bits(uint8 g, uint8 r, uint8 b)
{
	uint8 i;
	
	R8_SPI0_CTRL_MOD |= RB_SPI_MOSI_OE; 	//MOSIʹ��
	for(i=0;i<8;i++)
	{
		if(g & 0x80 )
			SPI0_MasterDMATrans(&CODE1,1);
		else
			SPI0_MasterDMATrans(&CODE0,1);
		
		g<<=1;
	}
	
	for(i=0;i<8;i++)
	{
		if(r & 0x80 )
			SPI0_MasterDMATrans(&CODE1,1);
		else
			SPI0_MasterDMATrans(&CODE0,1);
		
		r<<=1;
	}
	
	for(i=0;i<8;i++)
	{
		if(b & 0x80 )
			SPI0_MasterDMATrans(&CODE1,1);
		else
			SPI0_MasterDMATrans(&CODE0,1);
		
		b<<=1;
	}
	
}
/*******************************************************************************
 * @fn      RGB_24Bit
 *
 * @brief   WS2812 ���Ͷ���ƹ����� num*24bit
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void RGB_24Bit(uint32 num, uint8 r, uint8 g, uint8 b)
{
	uint32 i;
	for( i=0;i<num;i++ )
	{
		RGB_LED_Write_24Bits(g, r, b);
	}
	RGB_LED_Reset();
}

/*******************************************************************************
 * @fn      RGB_24Bit_Display1
 *
 * @brief   WS2812 �ƹ�Ч��1��ʾ����
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void RGB_24Bit_Display1(uint8_t *rgb)
{
	uint32 i;	
	for( i=0;i<WS2812_NUM*3;i++ )
	{
		RGB_LED_Write_24Bits(rgb[i*3], rgb[i*3+1], rgb[i*3+2]);
		
		if(rgb[i*3] != 0)
		{
			if(rgb[i*3] <= COLOR_MAX/7)
				rgb[i*3]=0x00;
			else
				rgb[i*3]-=COLOR_MAX/7;					// �ƹ�Ϩ�� ��ֵԽ��Ϩ��Խ��
		}
		if(rgb[i*3+1] != 0)
		{
			if(rgb[i*3+1] <= COLOR_MAX/7)
				rgb[i*3+1]=0x00;
			else
				rgb[i*3+1]-=COLOR_MAX/7;
			
		}
		if(rgb[i*3+2] != 0)
		{	
			if(rgb[i*3+2] <= COLOR_MAX/7)
				rgb[i*3+2]=0x00;
			else
				rgb[i*3+2]-=COLOR_MAX/7;
		}
	}
	RGB_LED_Reset();
}


/*******************************************************************************
 * @fn      WS2812_Init
 *
 * @brief   WS2812 �¼���ʼ��
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void WS2812_Init(void)
{
	/* RGB GPIO��ʼ�� */ 
	RGB_GPIO_Init();
	
	/* ע������ */ 
	WS2812_Taskid = TMOS_ProcessEventRegister(WS2812_ProcessEvent);
	
	
	/* ��ʼ WS2812 ���� */ 
	tmos_start_task( WS2812_Taskid , WS2812_EVENT_START ,1600 );
}

/*  
ROW1		14
ROW2		14
ROW3		13
ROW4		14
ROW5		9
*/
/*******************************************************************************
 * @fn      WS2812_Display_1
 *
 * @brief   WS2812 �ƹ�Ч��1
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void WS2812_Display_1()
{
	uint8_t col_temp=0, i=0;
	
//	rgb_record_temp[row_scanf_num][col_scanf_num] = 1;

//			/*  ROW 0 ��  */
//		for(col_temp=0;col_temp< 5; col_temp++)
//		{
//			if( rgb_record[0][col_temp] == 1 )
//			{
//				rgb_buf[((0*ROW_NUM+ 4-col_temp)*3)+1] = COLOR_MAX;
//			}
//		}
		i=0;
		for(col_temp=0;col_temp< 14; col_temp++)
		{
			if(Keyboard_Map_Temp[2][col_temp] != 0x00)
			{
				i++;
				if( rgb_record_temp[0][col_temp] == 1 )
				{
					rgb_buf[ ((0*14 + 13-col_temp)*3) + 1 ] = COLOR_MAX;
				}
			}
		}
		i=0;
		for(col_temp=0;col_temp< 14; col_temp++)
		{
			if(Keyboard_Map_Temp[2][col_temp] != 0x00)
			{
				i++;
				if( rgb_record_temp[1][col_temp] == 1 )
				{
					rgb_buf[ ((1*14 + 13-col_temp)*3) + 1 ] = COLOR_MAX;
				}
			}
		}
		
		i=0;
		for(col_temp=0;col_temp< 14; col_temp++)
		{
			if(Keyboard_Map_Temp[2][col_temp] != 0x00)
			{
				i++;
				if( rgb_record_temp[2][col_temp] == 1 )
				{
					rgb_buf[ ((1*14 + 1*14 + 13-i)*3) + 1 ] = COLOR_MAX;
				}
			}
		}
		
		i=0;
		for(col_temp=0;col_temp< 14; col_temp++)
		{
			if(Keyboard_Map_Temp[3][col_temp] != 0x00)
			{
				i++;
				if( rgb_record_temp[3][col_temp] == 1 )
				{
					rgb_buf[ ((1*13+1*14 + 1*14 + 14-i)*3) + 1 ] = COLOR_MAX;
				}
			}
		}
		i=0;
		for(col_temp=0;col_temp< 14; col_temp++)
		{
			if(Keyboard_Map_Temp[4][col_temp] != 0x00)
			{
				i++;
				if( rgb_record_temp[4][col_temp] == 1 )
				{
					rgb_buf[ ((1*14+1*13+1*14 + 1*14 + 9-i)*3) + 1 ] = COLOR_MAX;
				}
			}
		}
		
	RGB_24Bit_Display1(rgb_buf);
}

/*******************************************************************************
 * @fn      WS2812_ProcessEvent
 *
 * @brief   WS2812 �ƹ��¼�
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
tmosEvents WS2812_ProcessEvent( tmosTaskID task_id, tmosEvents events )
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
  if( events & WS2812_EVENT_START ){
		
		if(keyboard_mode_rgb_flag == BLE_MODE)						// �л�ģʽ�ƹ� BLE 
		{
			tmos_memset(rgb_buf, 0x00, sizeof(rgb_buf));		//���RGB����
			
			RGB_24Bit(WS2812_NUM, 0x00, 0xff, 0x00 );
		}else if(keyboard_mode_rgb_flag == USB_MODE)			// �л�ģʽ�ƹ� USB 
		{
			tmos_memset(rgb_buf, 0x00, sizeof(rgb_buf));		//���RGB����
			
			RGB_24Bit(WS2812_NUM, 0x00, 0x00, 0xff );
		}else
		{
			WS2812_Display_1();   //�ƹ�Ч��1
		}
//		RGB_24Bit(WS2812_NUM, rgb_r, rgb_g, rgb_b );
    tmos_start_task( WS2812_Taskid , WS2812_EVENT_START ,80 ); //  625us * time  100ms  
		return events ^ WS2812_EVENT_START;
  }
  return 0;
}
