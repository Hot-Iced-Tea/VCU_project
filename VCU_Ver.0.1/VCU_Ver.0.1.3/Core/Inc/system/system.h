/*
 * system.h
 *
 *  Created on: Aug 26, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_SYSTEM_SYSTEM_H_
#define INC_SYSTEM_SYSTEM_H_

#include "main.h"
#include "system/power_state/sleep_state.h"
#include "system/modes_state/standby_state.h"
#include "system/modes_state/charge_state.h"

extern volatile uint8_t authenticated;
extern volatile uint8_t charger;
extern volatile uint8_t CAN_lowBrake;
extern volatile uint8_t CAHN_HighBrake;

/* ========================================================= */
/* ===================== SYSTEM STATE ====================== */
/* ========================================================= */

typedef enum{
	SYSTEM_SLEEP = 0,
	SYSTEM_STANDBY,
	SYSTEM_RIDE,
	SYSTEM_REGISTRATION,
	SYSTEM_CHARGE,
	SYSTEM_FAULT,
}systemState_t;

extern systemState_t chargeReturnState;
/* ========================================================= */
/* ===================== PUBLIC FUNCTION =================== */
/* ========================================================= */

//initialize system state
void system_init(void);
void system_task(void);

//change current system state
void system_setState(systemState_t newState);

//get current system state
systemState_t system_GetState(void);

/* State initialization */
void system_sleepInit(void);
void system_standbyInit(void);
void system_rideInit(void);
void system_chargeInit(void);


/* State task */
void system_sleepTask(void);
void system_standbyTask(void);
void system_rideTask(void);
void system_chargeTask(void);

#endif /* INC_SYSTEM_SYSTEM_H_ */
