/*
 * steeringLock_control.h
 *
 *  Created on: Sep 9, 2026
 *      Author: Think
 */

#ifndef INC_IO_CONTROL_STEERINGLOCK_CONTROL_H_
#define INC_IO_CONTROL_STEERINGLOCK_CONTROL_H_

#include "main.h"
#include "stm32h5xx_hal.h"

/* ===== Command ===== */
#define steeringLock_cmd_Lock	0xAAAA
#define steeringLock_cmd_Unlock 0x5555

/* ===== Status Enum ===== */
typedef enum
{
    steeringLock_STATUS_UNKNOWN = 0,
    steeringLock_STATUS_UNLOCK  = 1,
    steeringLock_STATUS_LOCK    = 2,
    steeringLock_STATUS_MOVING  = 3,
    steeringLock_STATUS_ERROR   = 0xFF
} steeringLock_Status_t;

extern volatile uint8_t steeringLock_status;

void steeringLock_Init(void);

steeringLock_Status_t steeringLock_requestLock(void);
steeringLock_Status_t steeringLock_requestUnlock(void);
steeringLock_Status_t steeringLock_Lock(void);
steeringLock_Status_t steeringLock_Unlock(void);
steeringLock_Status_t steeringLock_GetStatus(uint16_t cmd);
void steeringLock_Task(void);

#endif /* INC_IO_CONTROL_STEERINGLOCK_CONTROL_H_ */
