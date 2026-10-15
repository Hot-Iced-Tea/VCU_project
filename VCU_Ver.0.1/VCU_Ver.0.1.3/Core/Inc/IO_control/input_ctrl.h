/*
 * input_ctrl.h
 *
 *  Created on: Sep 13, 2026
 *      Author: Think
 */

#ifndef INC_IO_CONTROL_INPUT_CTRL_H_
#define INC_IO_CONTROL_INPUT_CTRL_H_

#include "main.h"
#include "cfg/GPIO_config.h"


GPIO_PinState input_getState(const GPIO_Config *input);


#endif /* INC_IO_CONTROL_INPUT_CTRL_H_ */
