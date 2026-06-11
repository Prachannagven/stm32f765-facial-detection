#ifndef MY_USB_H
#define MY_USB_H

#include "stm32f7xx_hal.h"
#include <stdint.h>

#define USB_BUF_SIZE 128
HAL_StatusTypeDef USB_Print(const char* fmt, ...);

#endif
