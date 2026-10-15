/*
 * output_ctrl.h
 *
 *  Created on: Sep 11, 2026
 *      Author: Think
 */

#ifndef INC_IO_CONTROL_OUTPUT_CTRL_H_
#define INC_IO_CONTROL_OUTPUT_CTRL_H_

#include "main.h"
#include "cfg/GPIO_config.h"

void output_setState(const GPIO_Config *output, GPIO_PinState state);
GPIO_PinState output_getState(const GPIO_Config *output);


#endif /* INC_IO_CONTROL_OUTPUT_CTRL_H_ */
