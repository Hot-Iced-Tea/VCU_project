/*
 * GPIO.h
 *
 *  Created on: Jul 27, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_CFG_GPIO_H_
#define INC_CFG_GPIO_H_

#include "main.h"
#include <string.h>
//void OUT1_SET(void);
//void OUT1_RESET(void);
//void OUT1_TOGGLE(void);
//
//void OUT2_SET(void);
//void OUT2_RESET(void);
//void OUT2_TOGGLE(void);
//
//void OUT3_SET(void);
//void OUT3_RESET(void);
//void OUT3_TOGGLE(void);
//
//void GPIO_delay(uint32_t delay_ms);

void OUT_SET(uint16_t pin_set);
void OUT_RESET(uint16_t pin_reset);
void OUT_TOGGLE(uint16_t pin_toggle);

void GPIO_delay(uint32_t delay_ms);

void Blinky(void);
void locked_status(void);

#endif /* INC_CFG_GPIO_H_ */
