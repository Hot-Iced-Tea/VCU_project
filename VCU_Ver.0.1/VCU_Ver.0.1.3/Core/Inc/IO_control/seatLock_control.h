/*
 * seatLock_control.h
 *
 *  Created on: Sep 10, 2026
 *      Author: Think
 */

#ifndef INC_IO_CONTROL_SEATLOCK_CONTROL_H_
#define INC_IO_CONTROL_SEATLOCK_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stm32h5xx_hal.h"

#define seatLock_cmd_Trig 0xAAU

void seatLock_Init(void);
void seatLock_triggerOpen(void);
void seatLock_task(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_IO_CONTROL_SEATLOCK_CONTROL_H_ */
