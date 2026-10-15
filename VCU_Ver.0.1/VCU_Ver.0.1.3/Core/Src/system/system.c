/*
 * system.c
 *
 *  Created on: Aug 26, 2026
 *      Author: fahmif_imoto
 */

#include "system/system.h"
#include "IO_control/steeringLock_control.h"
#include "e_key/RFID/RFID_handler.h"

systemState_t chargeReturnState;

volatile uint8_t authenticated = 0;
volatile uint8_t charger = 0;
volatile uint8_t CAN_lowBrake = 1;
/* ========================================================= */
/* PRIVATE VARIABLE ======================================== */
/* ========================================================= */

static systemState_t currentState;

/* ========================================================= */
/* INITIALIZATION ========================================== */
/* ========================================================= */
void system_init(void){
	steeringLock_Init();
	RFID_handleInit();
	currentState = SYSTEM_SLEEP;
	system_sleepInit();
}

/* ========================================================= */
/* SYSTEM TASK ============================================= */
/* ========================================================= */
void system_task(void){
	steeringLock_Task();
	RFID_handleTask();

	switch(currentState){
	case SYSTEM_SLEEP		: system_sleepTask();
		break;
	case SYSTEM_STANDBY		: system_standbyTask();
		break;
	case SYSTEM_REGISTRATION: //system_registTask();
		break;
	case SYSTEM_RIDE		: system_rideTask();
		break;
	case SYSTEM_CHARGE		: system_chargeTask();
		break;
	case SYSTEM_FAULT		://system_faultTask();
		break;
	default					: currentState = SYSTEM_STANDBY;
		break;

	}
}

/* ========================================================= */
/* SET STATE SYSTEM ======================================== */
/* ========================================================= */
void system_setState(systemState_t newState)
{
    //case = going to charge state
    if (newState == SYSTEM_CHARGE){
        chargeReturnState = currentState; //save previous state before going to charge state
    }

    if (newState == currentState){
        return;
    }

    currentState = newState;

    //Initialize system state
    switch (currentState)
    {
        case SYSTEM_SLEEP		:system_sleepInit();
            break;
        case SYSTEM_STANDBY		:system_standbyInit();
            break;
        case SYSTEM_REGISTRATION: //system_registInit();
            break;
        case SYSTEM_RIDE		: system_rideInit();
            break;
        case SYSTEM_CHARGE		: system_chargeInit();
            break;
        case SYSTEM_FAULT		: //system_faulInit();
            break;
        default:
            break;
    }
}

systemState_t system_GetState(void){
	return currentState;
}
