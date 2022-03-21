/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/15
* Description        : �Զ���USB�豸��CH372�豸�����ṩ8����0ͨ��(�ϴ�+�´�)��ʵ���������´���Ȼ����������ȡ���ϴ�
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
/* USB����ID*/ 
uint8_t Usbhid_TaskId;

#define     DevEP0SIZE					0x40

#define     HIDCONFIGDESCLEN			0x49

#define     GAMEPAD_SIZ_REPORT_DESC			     61
#define     KEYBOARD_SIZ_REPORT_DESC    		 67
#define     MOUSE_SIZ_REPORT_DESC    			 62	

uint16_t UsbHid_ProcessEvent( uint8_t task_id, uint16_t events );


/* HID�ı���������*/
/*������8�ֽڷ��ͣ�
**  ��һ�ֽڱ�ʾ������Ƿ��£�D0:Ctrl D1:Shift D2:Alt
**  �ڶ��ֽڱ�����ֵΪ0
**  ����~�ڰ��ֽ�:��ͨ����ֵ������,�����ͬʱ����6����
**������1�ֽڽ���:��Ӧ�����ϵ�LED��,����ֻ��������λ��
**	D0:Num Lock   D1:Cap Lock   D2:Scroll Lock   D3:Compose   D4:Kana*/
const unsigned char  HIDREPORTDESC[]=
{
	/*short Item   D7~D4:bTag;D3~D2:bType;D1~D0:bSize
	**bTag ---����Ŀ  	1000:����(Input) 1001:���(Output) 1011:����(Feature)	1010:����(Collection) 1100:�ؼ���(End Collection) 
	**		  ȫ����Ŀ 	0000:��;ҳ(Usage Page) 0001:�߼���Сֵ(Logical Minimum) 0010:�߼����ֵ(Logical Maximum) 0011:������Сֵ(Physical Minimum)
	**					0100:�������ֵ(Physical Maximum) 0101:��Ԫָ��(Unit Exponet) 0110:��Ԫ(Unit) 0111:�������С(Report Size)
	**					1000:����ID(Report ID) 1001:����������(Report Count) 1010:ѹջ(Push) 1011:��ջ(Pop) 1100~1111:����(Reserved)
	**		  �ֲ���Ŀ	0000:��;(Usage) 0001:��;��Сֵ(Usage Minimum) 0010:��;���ֵ(Usage Maximum) 0011:��ʶ������(Designator Index)
	**					0100:��ʶ����Сֵ(Designator Minimum) 0101:��ʶ�����ֵ(Designator Maximum) 0111:�ַ�������(String Index) 1000:�ַ�����Сֵ(String Minimum)   
	**					1001:�ַ������ֵ(String Maximum) 1010:�ָ���(Delimiter) ����������(Reserved)
	**bType---00:����Ŀ(main)  01:ȫ����Ŀ(globle)  10:�ֲ���Ŀ(local)  11:����(reserved)
	**bSize---00:0�ֽ�  01:1�ֽ�  10:2�ֽ�  11:4�ֽ�*/
	
	//0x05:0000 01 01 ���Ǹ�ȫ����Ŀ����;ҳѡ��Ϊ��ͨ����ҳ
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	//0x09:0000 10 01 ���Ǹ�ȫ����Ŀ����;ѡ��Ϊ����
	0x09, 0x06, // USAGE (Keyboard)
	//0xa1:1010 00 01 ���Ǹ�����Ŀ��ѡ��ΪӦ�ü��ϣ�
	0xa1, 0x01, // COLLECTION (Application)
	//0x05:0000 01 11 ���Ǹ�ȫ����Ŀ����;ҳѡ��Ϊ����/����
	0x05, 0x07, // USAGE_PAGE (Keyboard/Keypad)

	//0x19:0001 10 01 ���Ǹ��ֲ���Ŀ����;����СֵΪ0xe0����Ӧ�����ϵ���ctrl��
	0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
	//0x29:0010 10 01 ���Ǹ��ֲ���Ŀ����;�����ֵΪ0xe7����Ӧ�����ϵ���GUI(WIN)��
	0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
	//0x15:0001 01 01 ���Ǹ�ȫ����Ŀ��˵�����ݵ��߼�ֵ��СֵΪ0
	0x15, 0x00, // LOGICAL_MINIMUM (0)
	//0x25:0010 01 01 ���Ǹ�ȫ����Ŀ��˵�����ݵ��߼�ֵ���ֵΪ1
	0x25, 0x01, // LOGICAL_MAXIMUM (1)

	//0x95:1001 01 01 ���Ǹ�ȫ����Ŀ�������������Ϊ8��
	0x95, 0x08, // REPORT_COUNT (8)
	//0x75:0111 01 01 ���Ǹ�ȫ����Ŀ��ÿ��������ĳ���Ϊ1λ
	0x75, 0x01, // REPORT_SIZE (1)	   
	//0x81:1000 00 01 ���Ǹ�����Ŀ����8*1bit��������Ϊ���룬����Ϊ:Data,Var,Abs
	0x81, 0x02, // INPUT (Data,Var,Abs)

	//0x95:1001 01 01 ���Ǹ�ȫ����Ŀ,�����������Ϊ1��
	0x95, 0x01, // REPORT_COUNT (1)
	//0x75:0111 01 01 ���Ǹ�ȫ����Ŀ��ÿ��������ĳ���Ϊ8λ
	0x75, 0x08, // REPORT_SIZE (8)
	//0x81:1000 00 01 ���Ǹ�����Ŀ����1*8bit��������Ϊ���룬����Ϊ:Cnst,Var,Abs
	0x81, 0x03, // INPUT (Cnst,Var,Abs)

	//0x95:1001 01 01 ���Ǹ�ȫ����Ŀ�������������Ϊ6��
	0x95, 0x06, // REPORT_COUNT (6)
	//0x75:0111 01 01 ���Ǹ�ȫ����Ŀ��ÿ��������ĳ���Ϊ8λ
	0x75, 0x08, // REPORT_SIZE (8)
	//0x25:0010 01 01 ���Ǹ�ȫ����Ŀ���߼����ֵΪ255
	0x25, 0xFF, // LOGICAL_MAXIMUM (255)
	//0x19:0001 10 01 ���Ǹ��ֲ���Ŀ����;����СֵΪ0
	0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
	//0x29:0010 10 01 ���Ǹ��ֲ���Ŀ����;�����ֵΪ0x65
	0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
	//0x81:1000 00 01 ���Ǹ�����Ŀ����6*8bit����������Ϊ���룬����Ϊ����Ϊ:Data,Var,Abs
	0x81, 0x00, // INPUT (Data,Ary,Abs)

	//0x25:0010 01 01 ���Ǹ�ȫ����Ŀ���߼������ֵΪ1
	0x25, 0x01, // LOGICAL_MAXIMUM (1)
	//0x95:1001 01 01 ���Ǹ�ȫ����Ŀ�������������Ϊ2
	0x95, 0x02, // REPORT_COUNT (2)
	//0x75:0111 01 01 ���Ǹ�ȫ����Ŀ��ÿ��������ĳ���Ϊ1λ
	0x75, 0x01, // REPORT_SIZE (1)
	//0x05:0000 01 01 ���Ǹ�ȫ����Ŀ����;ҳѡ��ΪLEDҳ
	0x05, 0x08, // USAGE_PAGE (LEDs)
	//0x19:0001 10 01 ���Ǹ��ֲ���Ŀ����;����СֵΪ0x01,��Ӧ�����ϵ�Num Lock
	0x19, 0x01, // USAGE_MINIMUM (Num Lock)
	//0x29:0010 10 01 ���Ǹ��ֲ���Ŀ����;�����ֵΪ0x02,��Ӧ�����ϵ�Caps Lock
	0x29, 0x02, // USAGE_MAXIMUM (Caps Lock)
	//0x91:1001 00 01 ���Ǹ�����Ŀ����2*1bit����������Ϊ���������Ϊ:Data,Var,Abs
	0x91, 0x02, // OUTPUT (Data,Var,Abs)
 
	//0x95:1001 01 01 ���Ǹ�ȫ����Ŀ�������������Ϊ1��
	0x95, 0x01, // REPORT_COUNT (1)
	//0x75:0111 01 01 ���Ǹ�ȫ����Ŀ��ÿ��������ĳ���Ϊ6bit,������ǰ���2bit���1�ֽ�
	0x75, 0x06, // REPORT_SIZE (6)
	//0x91:1001 00 01 ���Ǹ�����Ŀ����1*6bit��������Ϊ���������Ϊ:Cnst,Var,Abs
	0x91, 0x03, // OUTPUT (Cnst,Var,Abs)

	0xc0        // END_COLLECTION
}; 


/* �豸������ */ 
const UINT8  MyDevDescr[] = 
{
    0x12, 		         					/*bLength�����ȣ��豸�������ĳ���Ϊ18�ֽ�*/
    0x01, 											/*bDescriptorType�����ͣ��豸�������ı����0x01*/
    0x00,                       /*bcdUSB����ʹ�õ�USB�汾Ϊ2.0*/
    0x02,
    0xEF,                       /*bDeviceClass���豸��ʹ�õ������*/
    0x02,                       /*bDeviceSubClass���豸��ʹ�õ��������*/
    0x01,                       /*bDeviceProtocol���豸��ʹ�õ�Э��*/
    0x40,                       /*bMaxPacketSize����������Ϊ8�ֽ�*/
    0x78,                       /*idVendor������IDΪ0x7788*/
    0x67,
    0x12,                      	/*idProduct����ƷIDΪ0x1122*/
    0x01,
    0x10,                       /*bcdDevice���豸�İ汾��Ϊ2.00*/
    0x01,
    1,                          /*iManufacturer:�����ַ���������*/
    2,                          /*iProduct����Ʒ�ַ���������*/
    3,                          /*iSerialNumber���豸�����к��ַ�������*/
    0x01                        /*bNumConfiguration���豸��1������*/
}; /* keyboard�豸������ */

/* �豸���� */ 
const UINT8  MyCfgDescr[] = 
{
    0x09, 		  	/*bLength�����ȣ��豸�ַ����ĳ���Ϊ9�ֽ�*/
    USB_CONFIGURATION_DESCRIPTOR_TYPE, 	/*bDescriptorType�����ͣ����������������ͱ��Ϊ0x2*/
    IAD_HID_CDC_SIZ_CONFIG_DESC,	   		/*wTotalLength���������������ܳ���Ϊ41�ֽ�*/    
    0x00,
    0x03,         	/*bNumInterfaces��������֧�ֵĽӿ�����1��*/
    0x01,         	/*bConfigurationValue�������õ�ֵ*/
    0x00,         	/*iConfiguration�������õ��ַ���������ֵ����ֵΪ0��ʾû���ַ���*/              
    0xC0,         	/* bmAttributes:�豸��һЩ���ԣ�0xc0��ʾ�Թ��磬��֧��Զ�̻���
						D7:��������Ϊ1��D6:�Ƿ��Թ��磬D5���Ƿ�֧��Զ�̻��ѣ�D4~D0����������Ϊ0*/
    0x96,         	/*�������ϻ�õ�������Ϊ100mA */
//    0x96,         /*MaxPower���豸��Ҫ�������ϻ�ȡ���ٵ�������λΪ2mA��0x96��ʾ300mA*/
	
	/*********************************IAD Descriptor*********************************/	//�����豸 IAD ģ��
    0x08,                   //bLength 						��������С
    0x0B,               		//bDescriptorType			IAD����������
    0x00,                   //bFirstInterface 		��ʼ�ӿ�
    0x01,                   //bInferfaceCount			�ӿ���
    0x03,                   //bFunctionClass:HID	���ʹ���
    0x00,                   //bFunctionSubClass		�����ʹ���
    0x00,                   //bFunctionProtocol		Э�����
    0x00,                   //iFunction						�����ַ�������

   /************** HID �ӿ�������****************/	  // ��Ҫhid ����������
	/* 09 */
    0x09,         	/*bLength�����ȣ��ӿ��������ĳ���Ϊ9�ֽ� */
    USB_INTERFACE_DESCRIPTOR_TYPE,/* bDescriptorType���ӿ�������������Ϊ0x4 */
    0x00,         	/*bInterfaceNumber���ýӿڵı��*/
    0x00,        		/*bAlternateSetting���ýӿڵı��ñ�� */
    0x02,         	/*bNumEndpoints���ýӿ���ʹ�õĶ˵���*/
    0x03,         	/*bInterfaceClass�ýӿ���ʹ�õ���ΪHID*/
    0x01,         	/*bInterfaceSubClass���ýӿ����õ����� 1=BOOT, 0=no boot */
    0x01,         	/*nInterfaceProtocol :�ýӿ�ʹ�õ�Э��0=none, 1=keyboard, 2=mouse */
    0,            	/*iInterface: �ýӿ��ַ��������� */

    /*****************HID������ ********************/
	/* 18 */
    0x09,         	/*bLength: HID�������ĳ���Ϊ9�ֽ� */
    HID_DESCRIPTOR_TYPE, /* bDescriptorType: HID������������Ϊ0x21 */
    0x10,         	/*bcdHID: HIDЭ��İ汾Ϊ1.1 */
    0x01,
    0x00,         		/*bCountryCode: ���Ҵ��� */				// 32:UK	33 : US
    0x01,         	/*bNumDescriptors: �¼�������������*/
    0x22,         	/*bDescriptorType���¼�������������*/
    JOYSTICK_SIZ_REPORT_DESC,/* wItemLength: ��һ���������ĳ���*/
    0x00,

    /********************EP1 IN����˵�������******************/
	/* 27 */
    0x07,         	/* bLength: �˵��������ĳ���Ϊ7�ֽ�*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: �˵�������������Ϊ0x05*/
    0x81,         	/* bEndpointAddress: �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵��*/               
    0x03,         	/* bmAttributes: �˵������ΪΪ�ж϶˵�.
			D0~D1��ʾ��������:0(���ƴ���),1(��ʱ����),2(��������),3(�жϴ���)
			�ǵ�ʱ����˵�:D2~D7:����Ϊ0
			��ʱ����˵㣺
			D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��)
			D4~D5��ʾ��;:0(���ݶ˵�),1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����,*/
    0x08,         	/* wMaxPacketSize: �ö˵�֧�ֵ���������Ϊ8�ֽ�*/
    0x00,
    0x0A,         	/* bInterval: ��ѯ���(32ms) */

	/********************EP1 OUT����˵�������*******************/
	  0x07,         	/* bLength: �˵��������ĳ���Ϊ7�ֽ�*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: �˵�������������Ϊ0x05*/
    0x01,         	/* bEndpointAddress: �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵��*/               
    0x03,         	/* bmAttributes: �˵������ΪΪ�ж϶˵�.
			D0~D1��ʾ��������:0(���ƴ���),1(��ʱ����),2(��������),3(�жϴ���)
			�ǵ�ʱ����˵�:D2~D7:����Ϊ0
			��ʱ����˵㣺
			D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��)
			D4~D5��ʾ��;:0(���ݶ˵�),1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����,*/
    0x01,         	/* wMaxPacketSize: �ö˵�֧�ֵ���������Ϊ�ֽ�*/
    0x00,
    0x0A,         	/* bInterval: ��ѯ���(32ms) */
   /* 41 */
	 
	    /************** IAR �ӿ�������****************/
		/* 08 */
		0x08,                   //bLength 						��������С
    0x0B,               		//bDescriptorType			IAD����������
    0x01,                   //bFirstInterface 		��ʼ�ӿ�
    0x02,                   //bInferfaceCount			�ӿ���
    0x02,                   //bFunctionClass:HID	���ʹ���
    0x02,                   //bFunctionSubClass		�����ʹ���
    0x01,                   //bFunctionProtocol		Э�����
    0x05,                   //iFunction						�����ַ�������
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
	  /******************** EP2 �˵�������******************/
	  /* 27 */
    0x07,         	/* bLength: �˵��������ĳ���Ϊ7�ֽ�*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: �˵�������������Ϊ0x05*/
    0x82,         	/* bEndpointAddress: �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵��*/               
    0x03,         	/* bmAttributes: �˵������ΪΪ�ж϶˵�.
			D0~D1��ʾ��������:0(���ƴ���),1(��ʱ����),2(��������),3(�жϴ���)
			�ǵ�ʱ����˵�:D2~D7:����Ϊ0
			��ʱ����˵㣺
			D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��)
			D4~D5��ʾ��;:0(���ݶ˵�),1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����,*/
    0x40,         	/* wMaxPacketSize: �ö˵�֧�ֵ���������Ϊ8�ֽ�*/
    0x00,
    0xFF,         	/* bInterval: ��ѯ���(32ms) */
    /************** CDC �ӿ�������****************/
	  /* 09 */
    0x09,         	/*bLength�����ȣ��ӿ��������ĳ���Ϊ9�ֽ� */
    USB_INTERFACE_DESCRIPTOR_TYPE,/* bDescriptorType���ӿ�������������Ϊ0x4 */
    0x02,         	/*bInterfaceNumber���ýӿڵı��*/
    0x00,        	/*bAlternateSetting���ýӿڵı��ñ�� */
    0x02,         	/*bNumEndpoints���ýӿ���ʹ�õĶ˵���*/
    0x0A,         	/*bInterfaceClass�ýӿ���ʹ�õ���ΪCDC*/
    0x00,         	/*bInterfaceSubClass���ýӿ����õ����� 1=BOOT, 0=no boot */
    0x00,         	/*nInterfaceProtocol :�ýӿ�ʹ�õ�Э��0=none, 1=keyboard, 2=mouse */
    0,            	/*iInterface: �ýӿ��ַ��������� */	
		    /******************** EP3 OUT �˵�������******************/
	/* 27 */
    0x07,         	/* bLength: �˵��������ĳ���Ϊ7�ֽ�*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: �˵�������������Ϊ0x05*/
    0x03,         	/* bEndpointAddress: �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵��*/               
    0x02,         	/* bmAttributes: �˵������ΪΪ�ж϶˵�.
			D0~D1��ʾ��������:0(���ƴ���),1(��ʱ����),2(��������),3(�жϴ���)
			�ǵ�ʱ����˵�:D2~D7:����Ϊ0
			��ʱ����˵㣺
			D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��)
			D4~D5��ʾ��;:0(���ݶ˵�),1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����,*/
    0x40,         	/* wMaxPacketSize: �ö˵�֧�ֵ���������Ϊ8�ֽ�*/
    0x00,
    0x00,         	/* bInterval: ��ѯ���(32ms) */
	    /******************** EP3 IN �˵�������******************/
	/* 27 */
    0x07,         	/* bLength: �˵��������ĳ���Ϊ7�ֽ�*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: �˵�������������Ϊ0x05*/
    0x83,         	/* bEndpointAddress: �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵��*/               
    0x02,         	/* bmAttributes: �˵������ΪΪ�ж϶˵�.
			D0~D1��ʾ��������:0(���ƴ���),1(��ʱ����),2(��������),3(�жϴ���)
			�ǵ�ʱ����˵�:D2~D7:����Ϊ0
			��ʱ����˵㣺
			D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��)
			D4~D5��ʾ��;:0(���ݶ˵�),1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����,*/
    0x40,         	/* wMaxPacketSize: �ö˵�֧�ֵ���������Ϊ8�ֽ�*/
    0x00,
    0x00,         	/* bInterval: ��ѯ���(32ms) */

}; 
// ����������
const UINT8  MyLangDescr[] = { 0x04, 0x03, 0x09, 0x04 };
// ������Ϣ
const UINT8  MyManuInfo[] = { 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };
// ��Ʒ��Ϣ
const UINT8  MyProdInfo[] = { 0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '7', 0, 'x', 0 };


/**********************************************************/
UINT8   Ready;
UINT8   DevConfig;
UINT8   SetupReqCode;
UINT16  SetupReqLen;
const UINT8 *pDescr;

/******** �û��Զ������˵�RAM ****************************************/
__align(4) UINT8 EP0_Databuf[64+64+64];	//ep0(64)+ep4_out(64)+ep4_in(64)
__align(4) UINT8 EP1_Databuf[64+64];	//ep1_out(64)+ep1_in(64)
__align(4) UINT8 EP2_Databuf[64+64];	//ep2_out(64)+ep2_in(64)
__align(4) UINT8 EP3_Databuf[64+64];	//ep3_out(64)+ep3_in(64)

/*******************************************************************************
 * @fn      USB_DevTransProcess
 *
 * @brief   USBͨ�š�����
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
		switch ( R8_USB_INT_ST & ( MASK_UIS_TOKEN | MASK_UIS_ENDP ) )     // �����������ƺͶ˵��
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
							case 0x0a:break;		//���һ��Ҫ��
							case 0x09:break;
							default: errflag = 0xFF;
						}
							
              //errflag = 0xFF;				/* �Ǳ�׼���� */
          }
          else                            /* ��׼���� */
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
												errflag = 0xFF;   // ��֧�ֵ��ַ���������
												break;
										}
										break;
									
									default :
										errflag = 0xff;
										break;
								}
								if( SetupReqLen>len )	SetupReqLen = len;		//ʵ�����ϴ��ܳ���
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
								if ( ( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )    // �˵�
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
										errflag = 0xFF;                                 // ��֧�ֵĶ˵�
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
				
				if( errflag == 0xff)		// �����֧��
				{
//					SetupReqCode = 0xFF;
                    R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;  // STALL
				}
				else
				{
					if( chtype & 0x80 )		// �ϴ�
					{
						len = (SetupReqLen>DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
						SetupReqLen -= len;
					}
					else	len = 0;		// �´�	
					
					R8_UEP0_T_LEN = len; 
                    R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;  // Ĭ�����ݰ���DATA1
				}				
				break;
			
			case UIS_TOKEN_IN:
                switch( SetupReqCode )
                {
                case USB_GET_DESCRIPTOR:
                    len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;  // ���δ��䳤��
                    memcpy( pEP0_DataBuf, pDescr, len );                    /* �����ϴ����� */
                    SetupReqLen -= len;
                    pDescr += len;
                    R8_UEP0_T_LEN = len;
                    R8_UEP0_CTRL ^= RB_UEP_T_TOG;                             // ��ת
                    break;
                case USB_SET_ADDRESS:
                    R8_USB_DEV_AD = (R8_USB_DEV_AD&RB_UDA_GP_BIT) | SetupReqLen;
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                default:
                    R8_UEP0_T_LEN = 0;                                      // ״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
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
				{                       // ��ͬ�������ݰ�������
					len = R8_USB_RX_LEN;
					DevEP1_OUT_Deal( len );
				}
				break;
			
			case UIS_TOKEN_IN | 1:
				R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
				break;
			
			case UIS_TOKEN_OUT | 2:
				if ( R8_USB_INT_ST & RB_UIS_TOG_OK ) 
				{                       // ��ͬ�������ݰ�������
					len = R8_USB_RX_LEN;
					DevEP2_OUT_Deal( len );
				}
				break;
			
			case UIS_TOKEN_IN | 2:
				R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
				break;
			
			case UIS_TOKEN_OUT | 3:
				if ( R8_USB_INT_ST & RB_UIS_TOG_OK ) 
				{                       // ��ͬ�������ݰ�������
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
		if ( R8_USB_MIS_ST & RB_UMS_SUSPEND ) {;}	// ����
		else		{;}								// ����
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
 * @brief   USB HID �豸��ʼ��
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
 * @brief   USB������
 *
 * @param   task_id : ����ID
 *          events  : �¼�
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
{ /* �û����Զ��� */
	UINT8 i;
	for(i=0; i<l; i++)
	{
		pEP1_IN_DataBuf[i] = buf[i];
	}
	
	DevEP1_IN_Deal(l);
}

void DevEP2_OUT_Deal( UINT8 l )		
{ /* �û����Զ��� */
	pEP2_IN_DataBuf[2] = 0xff;
	DevEP2_IN_Deal( l );
}

void DevEP3_OUT_Deal( UINT8 l )		// USB ���⴮�ڷ���
{ /* �û����Զ��� */
	pEP3_IN_DataBuf[7] = 0xff;
	DevEP3_IN_Deal(8);
}

void DevEP4_OUT_Deal( UINT8 l )
{ /* �û����Զ��� */
//	UINT8 i;
	
//	for(i=0; i<l; i++)
//	{
//		pEP4_IN_DataBuf[i] = ~pEP4_OUT_DataBuf[i];
//	}
//	DevEP4_IN_Deal( l );
}

void USB_IRQHandler (void)		/* USB�жϷ������,ʹ�üĴ�����1 */
{
	USB_DevTransProcess();
}


