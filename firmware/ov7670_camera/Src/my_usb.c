#include "my_usb.h"
#include "stm32f7xx_hal_def.h"
#include "usbd_cdc_if.h"
#include <stdarg.h>
#include <stdio.h>

HAL_StatusTypeDef USB_Print(const char* fmt, ...) {
    uint8_t usbTxBuf[USB_BUF_SIZE];

    va_list args;
    va_start(args, fmt);

    int len = vsnprintf((char*)usbTxBuf, sizeof(usbTxBuf), fmt, args);

    va_end(args);

    if (len < 0) {
        return HAL_ERROR;
    }
    if (len >= sizeof(usbTxBuf)) {
        len = sizeof(usbTxBuf) - 1;
    }

    uint32_t timeout = HAL_GetTick();

    while (CDC_Transmit_FS(usbTxBuf, len) == USBD_BUSY) {
        if ((HAL_GetTick() - timeout) > 100) {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}
