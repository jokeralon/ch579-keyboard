/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/15
* Description        : 自定义USB设备（CH372设备），提供8个非0通道(上传+下传)，实现数据先下传，然后数据内容取反上传
*******************************************************************************/

#include "CONFIG.h"
#include "CH57x_common.h"
#include "CH57xBLE_LIB.h"
#include "hidkbd.h"
#include "key.h"
#include "mykeyboard.h"

extern uint8 buf[8];
extern uint8 keyboard_mode_flag;

extern uint8_t rgb_r,rgb_g, rgb_b;
/* USB任务ID*/ 
uint8_t Usbhid_TaskId;

#define     DevEP0SIZE					0x40

#define     HIDCONFIGDESCLEN			0x49

#define     GAMEPAD_SIZ_REPORT_DESC			     61
#define     KEYBOARD_SIZ_REPORT_DESC    		 67
#define     MOUSE_SIZ_REPORT_DESC    			 62	

uint16_t UsbHid_ProcessEvent( uint8_t task_id, uint16_t events );


/* HID的报告描述符*/
/*定义了8字节发送：
**  第一字节表示特殊件是否按下：D0:Ctrl D1:Shift D2:Alt
**  第二字节保留，值为0
**  第三~第八字节:普通键键值的数组,最多能同时按下6个键
**定义了1字节接收:对应键盘上的LED灯,这里只用了两个位。
**	D0:Num Lock   D1:Cap Lock   D2:Scroll Lock   D3:Compose   D4:Kana*/
const unsigned char  HIDREPORTDESC[]=
{
	/*short Item   D7~D4:bTag;D3~D2:bType;D1~D0:bSize
	**bTag ---主条目  	1000:输入(Input) 1001:输出(Output) 1011:特性(Feature)	1010:集合(Collection) 1100:关集合(End Collection) 
	**		  全局条目 	0000:用途页(Usage Page) 0001:逻辑最小值(Logical Minimum) 0010:逻辑最大值(Logical Maximum) 0011:物理最小值(Physical Minimum)
	**					0100:物理最大值(Physical Maximum) 0101:单元指数(Unit Exponet) 0110:单元(Unit) 0111:数据域大小(Report Size)
	**					1000:报告ID(Report ID) 1001:数据域数量(Report Count) 1010:压栈(Push) 1011:出栈(Pop) 1100~1111:保留(Reserved)
	**		  局部条目	0000:用途(Usage) 0001:用途最小值(Usage Minimum) 0010:用途最大值(Usage Maximum) 0011:标识符索引(Designator Index)
	**					0100:标识符最小值(Designator Minimum) 0101:标识符最大值(Designator Maximum) 0111:字符串索引(String Index) 1000:字符串最小值(String Minimum)   
	**					1001:字符串最大值(String Maximum) 1010:分隔符(Delimiter) 其他：保留(Reserved)
	**bType---00:主条目(main)  01:全局条目(globle)  10:局部条目(local)  11:保留(reserved)
	**bSize---00:0字节  01:1字节  10:2字节  11:4字节*/
	
	//0x05:0000 01 01 这是个全局条目，用途页选择为普通桌面页
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	//0x09:0000 10 01 这是个全局条目，用途选择为键盘
	0x09, 0x06, // USAGE (Keyboard)
	//0xa1:1010 00 01 这是个主条目，选择为应用集合，
	0xa1, 0x01, // COLLECTION (Application)
	//0x05:0000 01 11 这是个全局条目，用途页选择为键盘/按键
	0x05, 0x07, // USAGE_PAGE (Keyboard/Keypad)

	//0x19:0001 10 01 这是个局部条目，用途的最小值为0xe0，对应键盘上的左ctrl键
	0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
	//0x29:0010 10 01 这是个局部条目，用途的最大值为0xe7，对应键盘上的有GUI(WIN)键
	0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
	//0x15:0001 01 01 这是个全局条目，说明数据的逻辑值最小值为0
	0x15, 0x00, // LOGICAL_MINIMUM (0)
	//0x25:0010 01 01 这是个全局条目，说明数据的逻辑值最大值为1
	0x25, 0x01, // LOGICAL_MAXIMUM (1)

	//0x95:1001 01 01 这是个全局条目，数据域的数量为8个
	0x95, 0x08, // REPORT_COUNT (8)
	//0x75:0111 01 01 这是个全局条目，每个数据域的长度为1位
	0x75, 0x01, // REPORT_SIZE (1)	   
	//0x81:1000 00 01 这是个主条目，有8*1bit数据域作为输入，属性为:Data,Var,Abs
	0x81, 0x02, // INPUT (Data,Var,Abs)

	//0x95:1001 01 01 这是个全局条目,数据域的数量为1个
	0x95, 0x01, // REPORT_COUNT (1)
	//0x75:0111 01 01 这是个全局条目，每个数据域的长度为8位
	0x75, 0x08, // REPORT_SIZE (8)
	//0x81:1000 00 01 这是个主条目，有1*8bit数据域作为输入，属性为:Cnst,Var,Abs
	0x81, 0x03, // INPUT (Cnst,Var,Abs)

	//0x95:1001 01 01 这是个全局条目，数据域的数量为6个
	0x95, 0x06, // REPORT_COUNT (6)
	//0x75:0111 01 01 这是个全局条目，每个数据域的长度为8位
	0x75, 0x08, // REPORT_SIZE (8)
	//0x25:0010 01 01 这是个全局条目，逻辑最大值为255
	0x25, 0xFF, // LOGICAL_MAXIMUM (255)
	//0x19:0001 10 01 这是个局部条目，用途的最小值为0
	0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
	//0x29:0010 10 01 这是个局部条目，用途的最大值为0x65
	0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
	//0x81:1000 00 01 这是个主条目，有6*8bit的数据域作为输入，属相为属性为:Data,Var,Abs
	0x81, 0x00, // INPUT (Data,Ary,Abs)

	//0x25:0010 01 01 这是个全局条目，逻辑的最大值为1
	0x25, 0x01, // LOGICAL_MAXIMUM (1)
	//0x95:1001 01 01 这是个全局条目，数据域的数量为2
	0x95, 0x02, // REPORT_COUNT (2)
	//0x75:0111 01 01 这是个全局条目，每个数据域的长度为1位
	0x75, 0x01, // REPORT_SIZE (1)
	//0x05:0000 01 01 这是个全局条目，用途页选择为LED页
	0x05, 0x08, // USAGE_PAGE (LEDs)
	//0x19:0001 10 01 这是个局部条目，用途的最小值为0x01,对应键盘上的Num Lock
	0x19, 0x01, // USAGE_MINIMUM (Num Lock)
	//0x29:0010 10 01 这是个局部条目，用途的最大值为0x02,对应键盘上的Caps Lock
	0x29, 0x02, // USAGE_MAXIMUM (Caps Lock)
	//0x91:1001 00 01 这是个主条目，有2*1bit的数据域作为输出，属性为:Data,Var,Abs
	0x91, 0x02, // OUTPUT (Data,Var,Abs)
 
	//0x95:1001 01 01 这是个全局条目，数据域的数量为1个
	0x95, 0x01, // REPORT_COUNT (1)
	//0x75:0111 01 01 这是个全局条目，每个数据域的长度为6bit,正好与前面的2bit组成1字节
	0x75, 0x06, // REPORT_SIZE (6)
	//0x91:1001 00 01 这是个主条目，有1*6bit数据域最为输出，属性为:Cnst,Var,Abs
	0x91, 0x03, // OUTPUT (Cnst,Var,Abs)

	0xc0        // END_COLLECTION
}; 


/* 设备描述符 */ 
const UINT8  MyDevDescr[] = 
{
    0x12, 		         					/*bLength：长度，设备描述符的长度为18字节*/
    0x01, 											/*bDescriptorType：类型，设备描述符的编号是0x01*/
    0x00,                       /*bcdUSB：所使用的USB版本为2.0*/
    0x02,
    0xEF,                       /*bDeviceClass：设备所使用的类代码*/
    0x02,                       /*bDeviceSubClass：设备所使用的子类代码*/
    0x01,                       /*bDeviceProtocol：设备所使用的协议*/
    0x40,                       /*bMaxPacketSize：最大包长度为8字节*/
    0x78,                       /*idVendor：厂商ID为0x7788*/
    0x67,
    0x12,                      	/*idProduct：产品ID为0x1122*/
    0x01,
    0x10,                       /*bcdDevice：设备的版本号为2.00*/
    0x01,
    1,                          /*iManufacturer:厂商字符串的索引*/
    2,                          /*iProduct：产品字符串的索引*/
    3,                          /*iSerialNumber：设备的序列号字符串索引*/
    0x01                        /*bNumConfiguration：设备有1种配置*/
}; /* keyboard设备描述符 */

/* 设备配置 */ 
const UINT8  MyCfgDescr[] = 
{
    0x09, 		  	/*bLength：长度，设备字符串的长度为9字节*/
    USB_CONFIGURATION_DESCRIPTOR_TYPE, 	/*bDescriptorType：类型，配置描述符的类型编号为0x2*/
    IAD_HID_CDC_SIZ_CONFIG_DESC,	   		/*wTotalLength：配置描述符的总长度为41字节*/    
    0x00,
    0x03,         	/*bNumInterfaces：配置所支持的接口数量1个*/
    0x01,         	/*bConfigurationValue：该配置的值*/
    0x00,         	/*iConfiguration：该配置的字符串的索引值，该值为0表示没有字符串*/              
    0xC0,         	/* bmAttributes:设备的一些特性，0xc0表示自供电，不支持远程唤醒
						D7:保留必须为1，D6:是否自供电，D5：是否支持远程唤醒，D4~D0：保留设置为0*/
    0x96,         	/*从总线上获得的最大电流为100mA */
//    0x96,         /*MaxPower：设备需要从总线上获取多少电流，单位为2mA，0x96表示300mA*/
	
	/*********************************IAD Descriptor*********************************/	//复合设备 IAD 模型
    0x08,                   //bLength 						描述符大小
    0x0B,               		//bDescriptorType			IAD描述符类型
    0x00,                   //bFirstInterface 		起始接口
    0x01,                   //bInferfaceCount			接口数
    0x03,                   //bFunctionClass:HID	类型代码
    0x00,                   //bFunctionSubClass		子类型代码
    0x00,                   //bFunctionProtocol		协议代码
    0x00,                   //iFunction						描述字符串索引

   /************** HID 接口描述符****************/	  // 需要hid 报告描述符
	/* 09 */
    0x09,         	/*bLength：长度，接口描述符的长度为9字节 */
    USB_INTERFACE_DESCRIPTOR_TYPE,/* bDescriptorType：接口描述符的类型为0x4 */
    0x00,         	/*bInterfaceNumber：该接口的编号*/
    0x00,        		/*bAlternateSetting：该接口的备用编号 */
    0x02,         	/*bNumEndpoints：该接口所使用的端点数*/
    0x03,         	/*bInterfaceClass该接口所使用的类为HID*/
    0x01,         	/*bInterfaceSubClass：该接口所用的子类 1=BOOT, 0=no boot */
    0x01,         	/*nInterfaceProtocol :该接口使用的协议0=none, 1=keyboard, 2=mouse */
    0,            	/*iInterface: 该接口字符串的索引 */

    /*****************HID描述符 ********************/
	/* 18 */
    0x09,         	/*bLength: HID描述符的长度为9字节 */
    HID_DESCRIPTOR_TYPE, /* bDescriptorType: HID的描述符类型为0x21 */
    0x10,         	/*bcdHID: HID协议的版本为1.1 */
    0x01,
    0x00,         		/*bCountryCode: 国家代号 */				// 32:UK	33 : US
    0x01,         	/*bNumDescriptors: 下级描述符的数量*/
    0x22,         	/*bDescriptorType：下级描述符的类型*/
    JOYSTICK_SIZ_REPORT_DESC,/* wItemLength: 下一集描述符的长度*/
    0x00,

    /********************EP1 IN输入端点描述符******************/
	/* 27 */
    0x07,         	/* bLength: 端点描述符的长度为7字节*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: 端点描述符的类型为0x05*/
    0x81,         	/* bEndpointAddress: 该端点(输入)的地址,D7:0(OUT),1(IN),D6~D4:保留,D3~D0:端点号*/               
    0x03,         	/* bmAttributes: 端点的属性为为中断端点.
			D0~D1表示传输类型:0(控制传输),1(等时传输),2(批量传输),3(中断传输)
			非等时传输端点:D2~D7:保留为0
			等时传输端点：
			D2~D3表示同步的类型:0(无同步),1(异步),2(适配),3(同步)
			D4~D5表示用途:0(数据端点),1(反馈端点),2(暗含反馈的数据端点),3(保留)，D6~D7:保留,*/
    0x08,         	/* wMaxPacketSize: 该端点支持的最大包长度为8字节*/
    0x00,
    0x0A,         	/* bInterval: 轮询间隔(32ms) */

	/********************EP1 OUT输出端点描述符*******************/
	  0x07,         	/* bLength: 端点描述符的长度为7字节*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: 端点描述符的类型为0x05*/
    0x01,         	/* bEndpointAddress: 该端点(输入)的地址,D7:0(OUT),1(IN),D6~D4:保留,D3~D0:端点号*/               
    0x03,         	/* bmAttributes: 端点的属性为为中断端点.
			D0~D1表示传输类型:0(控制传输),1(等时传输),2(批量传输),3(中断传输)
			非等时传输端点:D2~D7:保留为0
			等时传输端点：
			D2~D3表示同步的类型:0(无同步),1(异步),2(适配),3(同步)
			D4~D5表示用途:0(数据端点),1(反馈端点),2(暗含反馈的数据端点),3(保留)，D6~D7:保留,*/
    0x01,         	/* wMaxPacketSize: 该端点支持的最大包长度为字节*/
    0x00,
    0x0A,         	/* bInterval: 轮询间隔(32ms) */
   /* 41 */
	 
	    /************** IAR 接口描述符****************/
		/* 08 */
		0x08,                   //bLength 						描述符大小
    0x0B,               		//bDescriptorType			IAD描述符类型
    0x01,                   //bFirstInterface 		起始接口
    0x02,                   //bInferfaceCount			接口数
    0x02,                   //bFunctionClass:HID	类型代码
    0x02,                   //bFunctionSubClass		子类型代码
    0x01,                   //bFunctionProtocol		协议代码
    0x05,                   //iFunction						描述字符串索引
	/************** Descriptor of virtual com port ****************/
    /*Interface Descriptor*/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */
    /*Header Functional Descriptor*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x36,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,
    /*Call Management Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x36,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x03,   /* bmCapabilities: D0+D1 */
    0x02,   /* bDataInterface: 1 */
    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x36,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x06,   /* bmCapabilities */
    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x36,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */
	  /******************** EP2 端点描述符******************/
	  /* 27 */
    0x07,         	/* bLength: 端点描述符的长度为7字节*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: 端点描述符的类型为0x05*/
    0x82,         	/* bEndpointAddress: 该端点(输入)的地址,D7:0(OUT),1(IN),D6~D4:保留,D3~D0:端点号*/               
    0x03,         	/* bmAttributes: 端点的属性为为中断端点.
			D0~D1表示传输类型:0(控制传输),1(等时传输),2(批量传输),3(中断传输)
			非等时传输端点:D2~D7:保留为0
			等时传输端点：
			D2~D3表示同步的类型:0(无同步),1(异步),2(适配),3(同步)
			D4~D5表示用途:0(数据端点),1(反馈端点),2(暗含反馈的数据端点),3(保留)，D6~D7:保留,*/
    0x40,         	/* wMaxPacketSize: 该端点支持的最大包长度为8字节*/
    0x00,
    0xFF,         	/* bInterval: 轮询间隔(32ms) */
    /************** CDC 接口描述符****************/
	  /* 09 */
    0x09,         	/*bLength：长度，接口描述符的长度为9字节 */
    USB_INTERFACE_DESCRIPTOR_TYPE,/* bDescriptorType：接口描述符的类型为0x4 */
    0x02,         	/*bInterfaceNumber：该接口的编号*/
    0x00,        	/*bAlternateSetting：该接口的备用编号 */
    0x02,         	/*bNumEndpoints：该接口所使用的端点数*/
    0x0A,         	/*bInterfaceClass该接口所使用的类为CDC*/
    0x00,         	/*bInterfaceSubClass：该接口所用的子类 1=BOOT, 0=no boot */
    0x00,         	/*nInterfaceProtocol :该接口使用的协议0=none, 1=keyboard, 2=mouse */
    0,            	/*iInterface: 该接口字符串的索引 */	
		    /******************** EP3 OUT 端点描述符******************/
	/* 27 */
    0x07,         	/* bLength: 端点描述符的长度为7字节*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: 端点描述符的类型为0x05*/
    0x03,         	/* bEndpointAddress: 该端点(输入)的地址,D7:0(OUT),1(IN),D6~D4:保留,D3~D0:端点号*/               
    0x02,         	/* bmAttributes: 端点的属性为为中断端点.
			D0~D1表示传输类型:0(控制传输),1(等时传输),2(批量传输),3(中断传输)
			非等时传输端点:D2~D7:保留为0
			等时传输端点：
			D2~D3表示同步的类型:0(无同步),1(异步),2(适配),3(同步)
			D4~D5表示用途:0(数据端点),1(反馈端点),2(暗含反馈的数据端点),3(保留)，D6~D7:保留,*/
    0x40,         	/* wMaxPacketSize: 该端点支持的最大包长度为8字节*/
    0x00,
    0x00,         	/* bInterval: 轮询间隔(32ms) */
	    /******************** EP3 IN 端点描述符******************/
	/* 27 */
    0x07,         	/* bLength: 端点描述符的长度为7字节*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: 端点描述符的类型为0x05*/
    0x83,         	/* bEndpointAddress: 该端点(输入)的地址,D7:0(OUT),1(IN),D6~D4:保留,D3~D0:端点号*/               
    0x02,         	/* bmAttributes: 端点的属性为为中断端点.
			D0~D1表示传输类型:0(控制传输),1(等时传输),2(批量传输),3(中断传输)
			非等时传输端点:D2~D7:保留为0
			等时传输端点：
			D2~D3表示同步的类型:0(无同步),1(异步),2(适配),3(同步)
			D4~D5表示用途:0(数据端点),1(反馈端点),2(暗含反馈的数据端点),3(保留)，D6~D7:保留,*/
    0x40,         	/* wMaxPacketSize: 该端点支持的最大包长度为8字节*/
    0x00,
    0x00,         	/* bInterval: 轮询间隔(32ms) */

}; 
// 语言描述符
const UINT8  MyLangDescr[] = { 0x04, 0x03, 0x09, 0x04 };
// 厂家信息
const UINT8  MyManuInfo[] = { 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };
// 产品信息
const UINT8  MyProdInfo[] = { 0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '7', 0, 'x', 0 };


/**********************************************************/
UINT8   Ready;
UINT8   DevConfig;
UINT8   SetupReqCode;
UINT16  SetupReqLen;
const UINT8 *pDescr;

/******** 用户自定义分配端点RAM ****************************************/
__align(4) UINT8 EP0_Databuf[64+64+64];	//ep0(64)+ep4_out(64)+ep4_in(64)
__align(4) UINT8 EP1_Databuf[64+64];	//ep1_out(64)+ep1_in(64)
__align(4) UINT8 EP2_Databuf[64+64];	//ep2_out(64)+ep2_in(64)
__align(4) UINT8 EP3_Databuf[64+64];	//ep3_out(64)+ep3_in(64)

/*******************************************************************************
 * @fn      USB_DevTransProcess
 *
 * @brief   USB通信、配置
 *
 * @param   void
 *
 * @return  none
 *******************************************************************************/
void USB_DevTransProcess( void )
{
	UINT8  len, chtype;
	UINT8  intflag, errflag = 0;
	
	intflag = R8_USB_INT_FG;
	if( intflag & RB_UIF_TRANSFER )
	{
		switch ( R8_USB_INT_ST & ( MASK_UIS_TOKEN | MASK_UIS_ENDP ) )     // 分析操作令牌和端点号
		{
			case UIS_TOKEN_SETUP:
        R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
				len = R8_USB_RX_LEN;
        if ( len == sizeof( USB_SETUP_REQ ) ) 
				{
					SetupReqLen = pSetupReqPak->wLength;
          SetupReqCode = pSetupReqPak->bRequest;
					chtype = pSetupReqPak->bRequestType;
					
					len = 0;
          errflag = 0;
					if ( ( pSetupReqPak->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )   
          {
						switch(SetupReqCode){
							case 0x0a:break;		//这个一定要有
							case 0x09:break;
							default: errflag = 0xFF;
						}
							
              //errflag = 0xFF;				/* 非标准请求 */
          }
          else                            /* 标准请求 */
          {
						switch( SetupReqCode )  
						{
							case USB_GET_DESCRIPTOR:
							{							
								switch( ((pSetupReqPak->wValue)>>8) )
								{	
									case USB_DESCR_TYP_DEVICE:
										pDescr = MyDevDescr;
										len = MyDevDescr[0];
										break;
									
									case USB_DESCR_TYP_CONFIG:
										pDescr = MyCfgDescr;
										len = MyCfgDescr[2];
										break;
									case USB_DESCR_TYP_REPORT:
										switch( (pSetupReqPak->wIndex)&0xff )
										{
											case 0:
												pDescr = (unsigned char*)&HIDREPORTDESC;
												len = sizeof(HIDREPORTDESC);
												break;
											
											default:
												errflag = 0xFF;   // 
										}
												break;
									case USB_DESCR_TYP_STRING:
										switch( (pSetupReqPak->wValue)&0xff )
										{
											case 0:
												pDescr = MyLangDescr;
												len = MyLangDescr[0];
												break;
											case 1:
												pDescr = MyManuInfo;
												len = MyManuInfo[0];
												break;
											case 2:
												pDescr = MyProdInfo;
												len = MyProdInfo[0];
												break;
											default:
												errflag = 0xFF;   // 不支持的字符串描述符
												break;
										}
										break;
									
									default :
										errflag = 0xff;
										break;
								}
								if( SetupReqLen>len )	SetupReqLen = len;		//实际需上传总长度
								len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;  
								memcpy( pEP0_DataBuf, pDescr, len );   
								pDescr += len;
							}
								break;
							
							case USB_SET_ADDRESS:
								SetupReqLen = (pSetupReqPak->wValue)&0xff;
								break;
							
							case USB_GET_CONFIGURATION:
								pEP0_DataBuf[0] = DevConfig;
								if ( SetupReqLen > 1 )		SetupReqLen = 1;
								break;
							
							case USB_SET_CONFIGURATION:
								DevConfig = (pSetupReqPak->wValue)&0xff;
								break;
							
							case USB_CLEAR_FEATURE:
								if ( ( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )    // 端点
								{
									switch( (pSetupReqPak->wIndex)&0xff )
									{
									case 0x82:
										R8_UEP2_CTRL = (R8_UEP2_CTRL & ~( RB_UEP_T_TOG|MASK_UEP_T_RES )) | UEP_T_RES_NAK;
										break;
									case 0x02:
										R8_UEP2_CTRL = (R8_UEP2_CTRL & ~( RB_UEP_R_TOG|MASK_UEP_R_RES )) | UEP_R_RES_ACK;
										break;
									case 0x81:
										R8_UEP1_CTRL = (R8_UEP1_CTRL & ~( RB_UEP_T_TOG|MASK_UEP_T_RES )) | UEP_T_RES_NAK;
										break;
									case 0x01:
										R8_UEP1_CTRL = (R8_UEP1_CTRL & ~( RB_UEP_R_TOG|MASK_UEP_R_RES )) | UEP_R_RES_ACK;
										break;
									default:
										errflag = 0xFF;                                 // 不支持的端点
										break;
									}
								}
								else	errflag = 0xFF;    
								break;
							
							case USB_GET_INTERFACE:
								pEP0_DataBuf[0] = 0x00;
								 if ( SetupReqLen > 1 )		SetupReqLen = 1;
								break;
							
							case USB_GET_STATUS:
								pEP0_DataBuf[0] = 0x00;
								pEP0_DataBuf[1] = 0x00;
								if ( SetupReqLen > 2 )		SetupReqLen = 2;
								break;
							
							default:
								errflag = 0xff;
								break;
						}
					}
				}
				else	errflag = 0xff;
				
				if( errflag == 0xff)		// 错误或不支持
				{
//					SetupReqCode = 0xFF;
                    R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;  // STALL
				}
				else
				{
					if( chtype & 0x80 )		// 上传
					{
						len = (SetupReqLen>DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
						SetupReqLen -= len;
					}
					else	len = 0;		// 下传	
					
					R8_UEP0_T_LEN = len; 
                    R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;  // 默认数据包是DATA1
				}				
				break;
			
			case UIS_TOKEN_IN:
                switch( SetupReqCode )
                {
                case USB_GET_DESCRIPTOR:
                    len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;  // 本次传输长度
                    memcpy( pEP0_DataBuf, pDescr, len );                    /* 加载上传数据 */
                    SetupReqLen -= len;
                    pDescr += len;
                    R8_UEP0_T_LEN = len;
                    R8_UEP0_CTRL ^= RB_UEP_T_TOG;                             // 翻转
                    break;
                case USB_SET_ADDRESS:
                    R8_USB_DEV_AD = (R8_USB_DEV_AD&RB_UDA_GP_BIT) | SetupReqLen;
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                default:
                    R8_UEP0_T_LEN = 0;                                      // 状态阶段完成中断或者是强制上传0长度数据包结束控制传输
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                }				
				break;
			
			case UIS_TOKEN_OUT:
				len = R8_USB_RX_LEN;
				if(SetupReqCode == 0x09)
        {
					if(pEP0_DataBuf[0])
					{
							printf("Light on Num Lock LED!\n");
					}
					else if(pEP0_DataBuf[0] == 0)
					{
							printf("Light off Num Lock LED!\n");
					}
         }
				break;
			
			case UIS_TOKEN_OUT | 1:
				if ( R8_USB_INT_ST & RB_UIS_TOG_OK ) 
				{                       // 不同步的数据包将丢弃
					len = R8_USB_RX_LEN;
					DevEP1_OUT_Deal( len );
				}
				break;
			
			case UIS_TOKEN_IN | 1:
				R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
				break;
			
			case UIS_TOKEN_OUT | 2:
				if ( R8_USB_INT_ST & RB_UIS_TOG_OK ) 
				{                       // 不同步的数据包将丢弃
					len = R8_USB_RX_LEN;
					DevEP2_OUT_Deal( len );
				}
				break;
			
			case UIS_TOKEN_IN | 2:
				R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
				break;
			
			case UIS_TOKEN_OUT | 3:
				if ( R8_USB_INT_ST & RB_UIS_TOG_OK ) 
				{                       // 不同步的数据包将丢弃
					len = R8_USB_RX_LEN;
					DevEP3_OUT_Deal( len );
				}
				break;
			
			case UIS_TOKEN_IN | 3:
				R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
				break;
			
			case UIS_TOKEN_OUT | 4:
				if ( R8_USB_INT_ST & RB_UIS_TOG_OK ) 
				{   
					R8_UEP4_CTRL ^= RB_UEP_R_TOG;
					len = R8_USB_RX_LEN;
					DevEP4_OUT_Deal( len );
				}
				break;
			
			case UIS_TOKEN_IN | 4:
				R8_UEP4_CTRL ^=  RB_UEP_T_TOG;
				R8_UEP4_CTRL = (R8_UEP4_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
				break;
			
			default :
				break;
		}
		R8_USB_INT_FG = RB_UIF_TRANSFER;
	}
	else if( intflag & RB_UIF_BUS_RST )
	{
		R8_USB_DEV_AD = 0;
		R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
		R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
		R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
		R8_USB_INT_FG |= RB_UIF_BUS_RST;
	}
	else if( intflag & RB_UIF_SUSPEND )
	{
		if ( R8_USB_MIS_ST & RB_UMS_SUSPEND ) {;}	// 挂起
		else		{;}								// 唤醒
		R8_USB_INT_FG = RB_UIF_SUSPEND;
	}
	else
	{
		R8_USB_INT_FG = intflag;
	}
}
 
 


/*******************************************************************************
 * @fn      UsbHid_Init
 *
 * @brief   USB HID 设备初始化
 *
 * @param   void
 *
 * @return  none
 *******************************************************************************/
void UsbHid_Init( void)
{
		Usbhid_TaskId = TMOS_ProcessEventRegister(UsbHid_ProcessEvent);
    pEP0_RAM_Addr = EP0_Databuf;
    pEP1_RAM_Addr = EP1_Databuf;
    pEP2_RAM_Addr = EP2_Databuf;
    pEP3_RAM_Addr = EP3_Databuf;
    USB_DeviceInit();
    NVIC_EnableIRQ( USB_IRQn );  
    
    // Setup a delayed profile startup
    tmos_set_event( Usbhid_TaskId, START_DEVICE_EVT );
//    HAL_KEY_RegisterForKeys(task_id);
}

uint8_t  ctrl_data;

/*******************************************************************************
 * @fn      UsbHid_ProcessEvent
 *
 * @brief   USB任务处理
 *
 * @param   task_id : 任务ID
 *          events  : 事件
 *
 * @return  tmosEvents
 *******************************************************************************/
uint16_t UsbHid_ProcessEvent( uint8_t task_id, uint16_t events )
{
    PRINT("%s IN,evt=%X\n",__func__,events);
  
    if ( events & SYS_EVENT_MSG )
    {
        uint8 *pMsg;

        if ( (pMsg = tmos_msg_receive( Usbhid_TaskId )) != NULL )
        {
//            Usbhid_ProcessTMOSMsg( (tmos_event_hdr_t *)pMsg );

            // Release the TMOS message
            tmos_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & START_DEVICE_EVT )
    {
			if(keyboard_mode_flag == USB_MODE)
			{
				DevEP1_OUT_Deal(8);
				DevEP2_OUT_Deal(8);
			}
        tmos_start_task( Usbhid_TaskId, START_DEVICE_EVT, 64 );
        return ( events ^ START_DEVICE_EVT );
    }

//    if ( events & START_REPORT_EVT )
//    {   
//				tmos_start_task( Usbhid_TaskId, START_REPORT_EVT, 1600 );
//        return ( events ^ START_REPORT_EVT );
//    }
		
    return 0;

}

void DevEP1_OUT_Deal( UINT8 l )		// 
{ /* 用户可自定义 */
	UINT8 i;
	for(i=0; i<l; i++)
	{
		pEP1_IN_DataBuf[i] = buf[i];
	}
	
	DevEP1_IN_Deal(l);
}

void DevEP2_OUT_Deal( UINT8 l )		
{ /* 用户可自定义 */
	pEP2_IN_DataBuf[2] = 0xff;
	DevEP2_IN_Deal( l );
}

void DevEP3_OUT_Deal( UINT8 l )		// USB 虚拟串口发送
{ /* 用户可自定义 */
	pEP3_IN_DataBuf[7] = 0xff;
	DevEP3_IN_Deal(8);
}

void DevEP4_OUT_Deal( UINT8 l )
{ /* 用户可自定义 */
//	UINT8 i;
	
//	for(i=0; i<l; i++)
//	{
//		pEP4_IN_DataBuf[i] = ~pEP4_OUT_DataBuf[i];
//	}
//	DevEP4_IN_Deal( l );
}

void USB_IRQHandler (void)		/* USB中断服务程序,使用寄存器组1 */
{
	USB_DevTransProcess();
}


