/*
 * LTE_config.c
 *
 *  Created on: Sep 10, 2026
 *      Author: fahmif_imoto
 */

#include "cfg/LTE_config.h"
//#include "usart.h"

const UART_config LTE_UART		= {};

const GPIO_Config LTE_power 	= {};
const GPIO_Config LTE_pwrKey 		= {LTE_ON_GPIO_Port, LTE_ON_Pin};
const GPIO_Config LTE_wakeUp 	= {};
const GPIO_Config LTE_sleep 	= {};
const GPIO_Config LTE_boot 		= {};
const GPIO_Config LTE_disable 	= {};
const GPIO_Config LTE_reset 	= {};
