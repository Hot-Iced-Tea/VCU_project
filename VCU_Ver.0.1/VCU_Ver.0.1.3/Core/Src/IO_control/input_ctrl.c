/*
 * input_ctrl.c
 *
 *  Created on: Sep 13, 2026
 *      Author: Think
 */

#include "input_ctrl.h"


GPIO_PinState input_getState(const GPIO_Config *input){
	return HAL_GPIO_ReadPin(input->port, input->pin);
}
