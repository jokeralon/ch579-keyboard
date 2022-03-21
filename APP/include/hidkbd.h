/********************************** (C) COPYRIGHT *******************************
* File Name          : hidkbd.h
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/10
* Description        : 
*******************************************************************************/

#ifndef HIDKBD_H
#define HIDKBD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// Task Events
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define JOYSTICK_SIZ_HID_DESC                   0x09
#define JOYSTICK_OFF_HID_DESC                   0x12

#define JOYSTICK_SIZ_DEVICE_DESC                18
#define JOYSTICK_SIZ_CONFIG_DESC                41
#define IAD_HID_CDC_SIZ_CONFIG_DESC							115
#define JOYSTICK_SIZ_REPORT_DESC                61
#define JOYSTICK_SIZ_STRING_LANGID              4
#define JOYSTICK_SIZ_STRING_VENDOR              38
#define JOYSTICK_SIZ_STRING_PRODUCT             40
#define JOYSTICK_SIZ_STRING_SERIAL              26

#define STANDARD_ENDPOINT_DESC_SIZE             0x09
	
#define START_DEVICE_EVT                              0x0001
#define START_REPORT_EVT                              0x0002
#define START_PARAM_UPDATE_EVT                        0x0004

#define START_GAMEPAD_REPORT_EVT                      0x0004
#define START_KEYBOARD_REPORT_EVT                     0x0008
#define START_MOUSE_REPORT_EVT                        0x0010
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */
  
/*********************************************************************
 * GLOBAL VARIABLES
 */

/*
 * Task Initialization for the BLE Application
 */
extern void HidEmu_Init( void );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 HidEmu_ProcessEvent( uint8 task_id, uint16 events );

extern void UsbHid_Init( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
