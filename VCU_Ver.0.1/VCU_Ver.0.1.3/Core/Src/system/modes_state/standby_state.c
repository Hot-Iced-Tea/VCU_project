/*
 * standby_state.c
 *
 *  Created on: Sep 10, 2026
 *      Author: fahmif_imoto
 */

#include "cfg/GPIO_config.h"
#include "IO_control/input_ctrl.h"
#include "IO_control/output_ctrl.h"
#include "IO_control/seatLock_control.h"
#include "IO_control/steeringLock_control.h"
#include "system/modes_state/standby_state.h"
#include "system/system.h"
#include "e_key/RFID/RFID_handler.h"

#include "main.h"
#include <stdint.h>



#define CONTROLLER_START_TIMEOUT_MS 15000U

typedef enum {
	STANDBY_NORMAL = 0,
	STANDBY_CONTROLLER_START
}standbySubState_t;

static standbySubState_t standbySubState = STANDBY_NORMAL;
static uint32_t controllerStartTick = 0U;

static void system_standbyNextState(void);
static void standby_controllerStart(void);
static void standby_controllerStartTask(void);

void system_standbyInit(void){
	standbySubState = STANDBY_NORMAL;
	controllerStartTick = 0U;



	output_setState(&ignition_rly, GPIO_PIN_RESET);
}

void system_standbyTask(void){
//	RFID_handleTask();
	switch (standbySubState){
	case STANDBY_NORMAL:
		RFID_handleTask();

		steeringLock_Task();
		system_standbyNextState();
		break;
	case STANDBY_CONTROLLER_START:
		standby_controllerStartTask();
		break;

	default:
		standbySubState = STANDBY_NORMAL;
		output_setState(&ignition_rly, GPIO_PIN_RESET);
		break;
	}
//	system_standbyNextState();
}

static void system_standbyNextState(void){
/* ========================================================= */
/* STANDBY -> SLEEP ======================================== */
/* ========================================================= */

//	if (RFID_handleIsAuthenticated())
//	if (RFID_handleGetAuthEvent() != 0U)
	if (authenticated == 1){
//		RFID_handleReset();
//		RFID_handleClearAuthentication();
//		RFID_handleClearAuthEvent();
		system_setState(SYSTEM_SLEEP);
		authenticated = 0;
		return;
	}

/* ========================================================= */
/* STANDBY -> CHARGE ======================================= */
/* ========================================================= */
	else if (charger == 1){
		system_setState(SYSTEM_CHARGE);
		return;
	}

/* ========================================================= */
/* STANDBY -> RIDE ========================================= */
/* ========================================================= */

	 else if ((input_getState(&sideStand_sw) == GPIO_PIN_SET) &&
	             (input_getState(&speedMode_sw) == GPIO_PIN_SET))
	 {
	        /*
	         * RIDE hanya boleh dimulai jika steering lock
	         * terdeteksi dalam kondisi UNLOCK.
	         */
	        if (steeringLock_status !=
	            (uint8_t)steeringLock_STATUS_UNLOCK)
	        {
	            output_setState(&ignition_rly, GPIO_PIN_RESET);
	            return;
	        }

	        standby_controllerStart();
	        return;
	    }

}

static void standby_controllerStart(void){
	output_setState(&ignition_rly, GPIO_PIN_SET);

	controllerStartTick = HAL_GetTick();

	standbySubState = STANDBY_CONTROLLER_START;
}

static void standby_controllerStartTask(void){
	volatile uint32_t elapsedTime;
	if (CAN_lowBrake == 0){
		standbySubState = STANDBY_NORMAL;
		system_setState(SYSTEM_RIDE);

		return;
	}

	elapsedTime = HAL_GetTick() - controllerStartTick;

	if (elapsedTime >= CONTROLLER_START_TIMEOUT_MS){
		output_setState(&ignition_rly, GPIO_PIN_RESET);
		controllerStartTick = 0U;
		standbySubState = STANDBY_NORMAL;
		return;
	}
}
