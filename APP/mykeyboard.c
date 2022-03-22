#include "mykeyboard.h"
#include "ws2812.h"
#include "hidkbd.h"
#include "string.h"
#include "app_adc.h"
//extern uint8 buf[HID_KEYBOARD_IN_RPT_LEN];
extern uint8 buf[8];

uint8 keyboard_mode_rgb_flag = USB_MODE;
uint8 keyboard_mode_flag = USB_MODE;
uint8 keyboard_stop_scnf_time = 0;

uint8 LShift_flag = 0;
uint8 RShift_flag = 0;
uint8 fn_flag = 0;
uint8 boot_flag = 0;
/* ws2812.h */
extern uint8_t rgb_r, rgb_g, rgb_b;
extern uint8_t rgb_record_temp[ROW_NUM][COL_NUM];

uint8 Keyboard_Taskid;
extern uint8_t WS2812_Taskid;
extern uint8_t Usbhid_TaskId;
extern uint8 hidEmuTaskId;

tmosEvents Keyboard_ProcessEvent(tmosTaskID task_id, tmosEvents events);

#define RGB_KEY 0xf1
#define FN_KEY 0xf2

const unsigned char myKeyBoard_FN_ATValue[12] =
    {
        0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45};

const char *Keyboard_Map_Name[ROW_NUM][COL_NUM] =
    {
        {"ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "BackSpace"},
        {"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|"},
        {"CapsLock", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", " ", "Enter"},
        {"LShift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "RShift", "UP", "DEL"},
        {"LCtrl", "LGui", "LAlt", " ", " ", "Space", " ", " ", " ", "RGB", "FN", "LEFT", "DOWN", "RGHT"}

};
/*  键值表  */
const uint8 Keyboard_Map_Temp[ROW_NUM][COL_NUM] =
    {
        {/*ESC*/ 0x29, /* 1 */ 0x1E, /* 2 */ 0x1f, /* 3 */ 0x20, /* 4 */ 0x21, /* 5 */ 0x22, /* 6 */ 0x23, /* 7 */ 0x24, /* 8 */ 0x25, /* 9 */ 0x26, /*0 0*/ 0x27, /* - */ 0x2D, /* + */ 0x2E, /*BCK*/ 0x2A},
        {/*TAB*/ 0x2B, /* Q */ 0x14, /* W */ 0x1A, /* E */ 0x08, /* R */ 0x15, /* T */ 0x17, /* Y */ 0x1C, /* U */ 0x18, /* I */ 0x0C, /* O */ 0x12, /* P */ 0x13, /* [ */ 0x2F, /* ] */ 0x30, /* \ */ 0x31},
        {/*CAP*/ 0x39, /* A */ 0x04, /* S */ 0x16, /* D */ 0x07, /* F */ 0x09, /* G */ 0x0A, /* H */ 0x0B, /* J */ 0x0D, /* K */ 0x0E, /* L */ 0x0F, /* ; */ 0x33, /* ' */ 0x34, /*NOP*/ 0x00, /*ENT*/ 0x58},
        {/*SHF*/ 0XE1, /* Z */ 0X1D, /* X */ 0X1B, /* C */ 0X06, /* V */ 0X19, /* B */ 0X05, /* N */ 0X11, /* M */ 0X10, /* , */ 0X36, /* . */ 0X37, /* / */ 0X38, /*SHF*/ 0XE5, /* UP*/ 0x52, /*DEL*/ 0X4C},
        {/*CTR*/ 0xE0, /*WIN*/ 0xE3, /*ALT*/ 0XE2, /*NOP*/ 0x00, /*NOP*/ 0x00, /*SPC*/ 0X2C, /*NOP*/ 0x00, /*NOP*/ 0x00, /*NOP*/ 0x00, /*RGB*/ 0XFF, /* FN*/ 0xF2, /*LFT*/ 0X50, /*DOW*/ 0X51, /*RIG*/ 0X4F}};

/*  按键缓存  */
uint8 Keyboard_temp[ROW_NUM][COL_NUM] = {0};

const uint32 Keyboard_COL_PIN[COL_NUM] =
    {
        COL1_PIN,
        COL2_PIN,
        COL3_PIN,
        COL4_PIN,
        COL5_PIN,
        COL6_PIN,
        COL7_PIN,
        COL8_PIN,
        COL9_PIN,
        COL10_PIN,
        COL11_PIN,
        COL12_PIN,
        COL13_PIN,
        COL14_PIN,
};
const uint8 Keyboard_COL_PORT[COL_NUM] =
    {
        COL1_PORT,
        COL2_PORT,
        COL3_PORT,
        COL4_PORT,
        COL5_PORT,
        COL6_PORT,
        COL7_PORT,
        COL8_PORT,
        COL9_PORT,
        COL10_PORT,
        COL11_PORT,
        COL12_PORT,
        COL13_PORT,
        COL14_PORT,
};

const uint32 Keyboard_ROW_PIN[ROW_NUM] =
    {
        ROW1_PIN,
        ROW2_PIN,
        ROW3_PIN,
        ROW4_PIN,
        ROW5_PIN,
};
const uint8 Keyboard_ROW_PORT[ROW_NUM] =
    {
        ROW1_PORT,
        ROW2_PORT,
        ROW3_PORT,
        ROW4_PORT,
        ROW5_PORT,
};
/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 驱动 GPIO初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void keyBoard_GPIO_SetMode(uint8 port, uint32 pin, GPIOModeTypeDef Mode)
{
    if (port)
    {
        GPIOB_ModeCfg(pin, Mode);
    }
    else
    {
        GPIOA_ModeCfg(pin, Mode);
    }
}

/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 驱动 GPIO初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
uint8 keyBoard_GPIO_ReadPin(uint8 port, uint32 pin)
{
    if (port)
    {
        if (GPIOB_ReadPortPin(pin) == 0)
            return 0;
        else
            return 1;
    }
    else
    {
        if (GPIOA_ReadPortPin(pin) == 0)
            return 0;
        else
            return 1;
    }
}
/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 驱动 GPIO初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void keyBoard_GPIO_SetPin(uint8 port, uint32 pin, uint8 status)
{
    if (port)
    {
        if (status == GPIO_HIGH)
            GPIOB_SetBits(pin);
        else
            GPIOB_ResetBits(pin);
    }
    else
    {
        if (status == GPIO_HIGH)
            GPIOA_SetBits(pin);
        else
            GPIOA_ResetBits(pin);
    }
}
/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 驱动 GPIO初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void keyBoard_GPIO_Init()
{
    keyBoard_GPIO_SetMode(ROW1_PORT, ROW1_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(ROW2_PORT, ROW2_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(ROW3_PORT, ROW3_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(ROW4_PORT, ROW4_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(ROW5_PORT, ROW5_PIN, GPIO_ModeOut_PP_5mA);

    keyBoard_GPIO_SetPin(ROW1_PORT, ROW1_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(ROW2_PORT, ROW2_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(ROW3_PORT, ROW3_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(ROW4_PORT, ROW4_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(ROW5_PORT, ROW5_PIN, GPIO_LOW);

    mDelaymS(10);
    keyBoard_GPIO_SetMode(COL1_PORT, COL1_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL2_PORT, COL2_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL3_PORT, COL3_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL4_PORT, COL4_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL5_PORT, COL5_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL6_PORT, COL6_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL7_PORT, COL7_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL8_PORT, COL8_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL9_PORT, COL9_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL10_PORT, COL10_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL11_PORT, COL11_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL12_PORT, COL12_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL13_PORT, COL13_PIN, GPIO_ModeOut_PP_5mA);
    keyBoard_GPIO_SetMode(COL14_PORT, COL14_PIN, GPIO_ModeOut_PP_5mA);
    mDelaymS(10);
    keyBoard_GPIO_SetPin(COL1_PORT, COL1_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL2_PORT, COL2_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL3_PORT, COL3_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL4_PORT, COL4_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL5_PORT, COL5_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL6_PORT, COL6_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL7_PORT, COL7_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL8_PORT, COL8_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL9_PORT, COL9_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL10_PORT, COL10_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL11_PORT, COL11_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL12_PORT, COL12_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL13_PORT, COL13_PIN, GPIO_LOW);
    keyBoard_GPIO_SetPin(COL14_PORT, COL14_PIN, GPIO_LOW);
    mDelaymS(10);
    keyBoard_GPIO_SetMode(COL1_PORT, COL1_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL2_PORT, COL2_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL3_PORT, COL3_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL4_PORT, COL4_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL5_PORT, COL5_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL6_PORT, COL6_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL7_PORT, COL7_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL8_PORT, COL8_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL9_PORT, COL9_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL10_PORT, COL10_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL11_PORT, COL11_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL12_PORT, COL12_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL13_PORT, COL13_PIN, GPIO_ModeIN_PD);
    keyBoard_GPIO_SetMode(COL14_PORT, COL14_PIN, GPIO_ModeIN_PD);

    keyBoard_GPIO_SetMode(GPIO_PortB, GPIO_Pin_22, GPIO_ModeIN_PU);
    mDelaymS(10);
}

/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 驱动 GPIO初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void keyBoard_Scanf(void)
{
    uint8 col_scanf_num = 0, row_scanf_num = 0;

    if (keyBoard_GPIO_ReadPin(GPIO_PortB, GPIO_Pin_22) == 0) //BOOT 键
    {
        boot_flag = 1;
        Keyboard_temp[4][9] = 1;
        rgb_record_temp[4][9] = 1;
    }

    for (row_scanf_num = 0; row_scanf_num < ROW_NUM; row_scanf_num++)
    {
        keyBoard_GPIO_SetPin(Keyboard_ROW_PORT[row_scanf_num], Keyboard_ROW_PIN[row_scanf_num], GPIO_HIGH);
        for (col_scanf_num = 0; col_scanf_num < COL_NUM; col_scanf_num++)
        {
            if (keyBoard_GPIO_ReadPin(Keyboard_COL_PORT[col_scanf_num], Keyboard_COL_PIN[col_scanf_num]) == 1)
            {
                rgb_record_temp[row_scanf_num][col_scanf_num] = 1;
                if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "FN") == 0) // FN
                  fn_flag = 1;
                else
								{
									if(strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LShift") == 0)
										LShift_flag = 1;
									if(strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "RShift") == 0)
										RShift_flag = 1;
									Keyboard_temp[row_scanf_num][col_scanf_num] = 1;
								}
            }
            else
            {
                Keyboard_temp[row_scanf_num][col_scanf_num] = 0;
            }
        }
        keyBoard_GPIO_SetPin(Keyboard_ROW_PORT[row_scanf_num], Keyboard_ROW_PIN[row_scanf_num], GPIO_LOW);
    }
}

/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 驱动 GPIO初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
uint8 keyBoard_SendValue(void)
{
    uint8 col_scanf_num = 0, row_scanf_num = 0;
    uint8 i = 2; //普通按键
    uint8 k = 0; //功能键
    tmos_memset(buf, 0x00, sizeof(buf));
    tmos_memset(Keyboard_temp, 0x00, sizeof(Keyboard_temp));
	
    LShift_flag = 0;
    RShift_flag = 0;

    keyBoard_Scanf();//键盘扫描
	
	
    for (row_scanf_num = 0; row_scanf_num < ROW_NUM; row_scanf_num++)
    {
        for (col_scanf_num = 0; col_scanf_num < COL_NUM; col_scanf_num++)
        {
            if (Keyboard_temp[row_scanf_num][col_scanf_num] == 1)
            {
                if (fn_flag == 1) // FN
                {
                    fn_flag = 0;
                    if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "1") == 0) //FN F1
                    {
                        buf[i] = myKeyBoard_FN_ATValue[0];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "2") == 0) //FN F2
                    {
                        buf[i] = myKeyBoard_FN_ATValue[1];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "3") == 0) //FN F3
                    {
                        buf[i] = myKeyBoard_FN_ATValue[2];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "4") == 0) //FN F4
                    {
                        buf[i] = myKeyBoard_FN_ATValue[3];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "5") == 0) //FN F5
                    {
                        buf[i] = myKeyBoard_FN_ATValue[4];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "6") == 0) //FN F6
                    {
                        buf[i] = myKeyBoard_FN_ATValue[5];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "7") == 0) //FN F7
                    {
                        buf[i] = myKeyBoard_FN_ATValue[6];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "8") == 0) //FN F8
                    {
                        buf[i] = myKeyBoard_FN_ATValue[7];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "9") == 0) //FN F9
                    {
                        buf[i] = myKeyBoard_FN_ATValue[8];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "0") == 0) //FN F10
                    {
                        buf[i] = myKeyBoard_FN_ATValue[9];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "-") == 0) //FN F11
                    {
                        buf[i] = myKeyBoard_FN_ATValue[10];
                        i++;
                    }
                    else if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "+") == 0) //FN F12
                    {
                        buf[i] = myKeyBoard_FN_ATValue[11];
                        i++;
                    }
                }
                else if (boot_flag == 1) // BOOT
                {
                    if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "1") == 0) //BOOT 1 切换 USB
                    {
                        keyboard_mode_flag = USB_MODE;
                        keyboard_mode_rgb_flag = USB_MODE;
                        tmos_stop_task(Usbhid_TaskId, START_DEVICE_EVT);
                        tmos_stop_task(hidEmuTaskId, START_REPORT_EVT);
                    }
                    if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "2") == 0) //BOOT 2 切换 BLE
                    {
                        keyboard_mode_flag = BLE_MODE;
                        keyboard_mode_rgb_flag = BLE_MODE;
                        tmos_stop_task(Usbhid_TaskId, START_DEVICE_EVT);
                        tmos_stop_task(hidEmuTaskId, START_REPORT_EVT);
											
												tmos_set_event(hidEmuTaskId, START_REPORT_EVT);
                    }
                }
                else if ((strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LShift") == 0) \
									|| (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LCtrl") == 0) \
									|| (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LGui") == 0) \
									|| (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LAlt") == 0) \
									|| (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "RShift") == 0))
                {
                    if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LShift") == 0) //LShift
                    {
                        buf[0] |= (0x01 << 1); //LShift
                        k++;
                    }
                    if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LCtrl") == 0) //LCtrl
                    {
                        buf[0] |= (0x01 << 0); //LCtrl
                        k++;
                    }
                    if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LGui") == 0) //LGui
                    {
                        buf[0] |= (0x01 << 3); //LGui
                        k++;
                    }
                    if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "LAlt") == 0) //LAlt
                    {
                        buf[0] |= (0x01 << 2); //LAlt
                        k++;
                    }
                    if (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "RShift") == 0) //RShift
                    {
                        buf[0] |= (0x01 << 1); //RShift
                        k++;
                    }
                }
                if (i < 6)
                {
                    /* 普通键处理 */
                    if ((Keyboard_Map_Temp[row_scanf_num][col_scanf_num] != 0xFF) && (Keyboard_Map_Temp[row_scanf_num][col_scanf_num] != 0xF2))
                    {
                        if ((LShift_flag == 1 || RShift_flag == 1) && (strcmp(Keyboard_Map_Name[row_scanf_num][col_scanf_num], "ESC") == 0))
                            buf[i] = 0x35; // 波浪键
                        else
                            buf[i] = Keyboard_Map_Temp[row_scanf_num][col_scanf_num];
                        i++;
                    }
                }
                //				if(i>7)
                //					return 1;
            }
            mDelayuS(1);
        }
    }
		 if( boot_flag == 1 )
		 {
				boot_flag = 0;
			  tmos_memset(buf, 0x00, sizeof(buf));
		 }
    if (k == 0)
        buf[0] = 0x00;

    return 0;
}

/*******************************************************************************
 * @fn      RGB_GPIO_Init
 *
 * @brief   WS2812 SPI0 驱动 GPIO初始化
 *
 * @param   void 
 *
 * @return  none
 *******************************************************************************/
void Keyboard_Init(void)
{
    /* 注册任务 */
    Keyboard_Taskid = TMOS_ProcessEventRegister(Keyboard_ProcessEvent);

    /* RGB GPIO初始化 */
    keyBoard_GPIO_Init();
    DelayMs(500);
    /* 开始 WS2812 任务 */
    tmos_start_task(Keyboard_Taskid, KEYBOARD_SCANF_EVENT, 1600);
}

tmosEvents Keyboard_ProcessEvent(tmosTaskID task_id, tmosEvents events)
{
    uint8 *msgPtr;

    if (events & SYS_EVENT_MSG)
    { // 处理HAL层消息，调用tmos_msg_receive读取消息，处理完成后删除消息。
        msgPtr = tmos_msg_receive(task_id);
        if (msgPtr)
        {
            /* De-allocate */
            tmos_msg_deallocate(msgPtr);
        }
        return events ^ SYS_EVENT_MSG;
    }
    if (events & KEYBOARD_SCANF_EVENT)
    {

        if (keyboard_mode_rgb_flag == RGB_NULL)
        {
            if (keyBoard_SendValue() == 0)
                ;
            else
                ;
        }
        else
        {
            keyboard_stop_scnf_time++;
            if (keyboard_stop_scnf_time == 25)
            {
                if (keyboard_mode_flag == USB_MODE)
                    tmos_set_event(Usbhid_TaskId, START_DEVICE_EVT);
                else
                    tmos_set_event(hidEmuTaskId, START_REPORT_EVT);
                keyboard_stop_scnf_time = 0;
                keyboard_mode_rgb_flag = RGB_NULL;
            }
        }

        tmos_start_task(Keyboard_Taskid, KEYBOARD_SCANF_EVENT, 32); //  625us / run  1000ms
        return events ^ KEYBOARD_SCANF_EVENT;
    }
    return 0;
}
