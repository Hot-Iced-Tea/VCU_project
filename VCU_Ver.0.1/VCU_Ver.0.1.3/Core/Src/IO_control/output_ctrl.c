/*
 * output_ctrl.c
 *
 *  Created on: Sep 11, 2026
 *      Author: Think
 */


#include "output_ctrl.h"

void output_setState(const GPIO_Config *output, GPIO_PinState state){
	HAL_GPIO_WritePin(output->port, output->pin, state);
}

GPIO_PinState output_getState(const GPIO_Config *output){
	return HAL_GPIO_ReadPin(output->port, output->pin);
}
