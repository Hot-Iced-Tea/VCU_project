/*
 * UART_config.h
 *
 *  Created on: Sep 10, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_CFG_UART_CONFIG_H_
#define INC_CFG_UART_CONFIG_H_

#include "main.h"
#include "stm32h5xx_hal.h"


typedef struct
{
    UART_HandleTypeDef *huart;
} UART_config;

extern const UART_config fingerPrint_UART;
extern const UART_config GNSS_UART;

#endif /* INC_CFG_UART_CONFIG_H_ */
