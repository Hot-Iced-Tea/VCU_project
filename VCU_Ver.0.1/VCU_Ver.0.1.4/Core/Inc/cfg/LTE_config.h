/*
 * LTE_config.h
 *
 *  Created on: Sep 10, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_CFG_LTE_CONFIG_H_
#define INC_CFG_LTE_CONFIG_H_

#include "main.h"
#include "GPIO_config.h"
#include "UART_config.h"

extern const UART_config LTE_UART;

extern const GPIO_Config LTE_power;
extern const GPIO_Config LTE_pwrKey;
extern const GPIO_Config LTE_wakeUp;
extern const GPIO_Config LTE_sleep;
extern const GPIO_Config LTE_boot;
extern const GPIO_Config LTE_disable;
extern const GPIO_Config LTE_reset;


#endif /* INC_CFG_LTE_CONFIG_H_ */
