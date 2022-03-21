#include "ws2812.h"
#include "mykeyboard.h"


extern uint8 keyboard_mode_flag;									// 模式标志
extern uint8 keyboard_mode_rgb_flag;							// 模式RGB标志
extern uint8 Keyboard_Map_Temp[ROW_NUM][COL_NUM];	// 按码表
uint8_t rgb_record_temp[ROW_NUM][COL_NUM];				// 实时按键表

uint8_t WS2812_Taskid = 0;												// WS2812 事件ID
unsigned char CODE1 = 0xFC;
unsigned char CODE0 = 0xC0;

void RGB_LED_Reset(void);
void RGB_LED_Write_24Bits(uint8 g, uint8 r, uint8 b);
tmosEvents WS2812_ProcessEvent( tmosTaskID task_id, tmosEvents events );

unsigned char reset[3] = {0,0,0};									// 复位数据
uint8_t rgb_buf[3*WS2812_NUM]={0x00};							// RGB 数据


/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 驱动 GPIO初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void RGB_GPIO_Init(void)
{
    GPIOA_ModeCfg(GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    SPI0_MasterDefInit( );									// SPI0 初始化 4分频
	
		RGB_LED_Write_24Bits(0X00,0X00,0X00);		
	
		// WS2812 复位	
		R8_SPI0_CTRL_MOD &= !RB_SPI_MOSI_OE;		// SPI0 MOSI 禁止
		R8_SPI0_CTRL_MOD &= !RB_SPI_SCK_OE;			// SPI0 SCK 禁止
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
 * @brief   WS2812 清除数据
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void RGB_LED_Reset(void)
{
//	RGB_LED_LOW;
	
	R8_SPI0_CTRL_MOD &= !RB_SPI_MOSI_OE; //MOSI 禁止
	mDelayuS(1);
	GPIOA_ModeCfg( GPIO_Pin_14, GPIO_ModeOut_PP_5mA );
	GPIOA_ResetBits( GPIO_Pin_14 );
	
	mDelayuS(80);
}
/*******************************************************************************
 * @fn      RGB_LED_Write_24Bits
 *
* @brief   WS2812 发送一个灯光数据 24bit
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void RGB_LED_Write_24Bits(uint8 g, uint8 r, uint8 b)
{
	uint8 i;
	
	R8_SPI0_CTRL_MOD |= RB_SPI_MOSI_OE; 	//MOSI使能
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
 * @brief   WS2812 发送多个灯光数据 num*24bit
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
 * @brief   WS2812 灯光效果1显示驱动
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
				rgb[i*3]-=COLOR_MAX/7;					// 灯光熄灭 数值越大熄灭越慢
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
 * @brief   WS2812 事件初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void WS2812_Init(void)
{
	/* RGB GPIO初始化 */ 
	RGB_GPIO_Init();
	
	/* 注册任务 */ 
	WS2812_Taskid = TMOS_ProcessEventRegister(WS2812_ProcessEvent);
	
	
	/* 开始 WS2812 任务 */ 
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
 * @brief   WS2812 灯光效果1
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void WS2812_Display_1()
{
	uint8_t col_temp=0, i=0;
	
//	rgb_record_temp[row_scanf_num][col_scanf_num] = 1;

//			/*  ROW 0 反  */
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
 * @brief   WS2812 灯光事件
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
tmosEvents WS2812_ProcessEvent( tmosTaskID task_id, tmosEvents events )
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
  if( events & WS2812_EVENT_START ){
		
		if(keyboard_mode_rgb_flag == BLE_MODE)						// 切换模式灯光 BLE 
		{
			tmos_memset(rgb_buf, 0x00, sizeof(rgb_buf));		//清空RGB缓存
			
			RGB_24Bit(WS2812_NUM, 0x00, 0xff, 0x00 );
		}else if(keyboard_mode_rgb_flag == USB_MODE)			// 切换模式灯光 USB 
		{
			tmos_memset(rgb_buf, 0x00, sizeof(rgb_buf));		//清空RGB缓存
			
			RGB_24Bit(WS2812_NUM, 0x00, 0x00, 0xff );
		}else
		{
			WS2812_Display_1();   //灯光效果1
		}
//		RGB_24Bit(WS2812_NUM, rgb_r, rgb_g, rgb_b );
    tmos_start_task( WS2812_Taskid , WS2812_EVENT_START ,80 ); //  625us * time  100ms  
		return events ^ WS2812_EVENT_START;
  }
  return 0;
}
