#ifndef __POWER_H
#define __POWER_H

#include "CONFIG.h"
#include "CH57x_common.h"
#include "CH57xBLE_LIB.h"

#include "usbhid.h"

#define LOW_POWER_SERVICE		0x0001

void LowPowerSerive(void);
void PowerManage_Init(void);
#endif
