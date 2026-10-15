/*
 * sleep_state.c
 *
 *  Created on: Sep 10, 2026
 *      Author: fahmif_imoto
 */

#include "cfg/GPIO_config.h"
#include "IO_control/output_ctrl.h"
#include "IO_control/steeringLock_control.h"
#include "system/power_state/sleep_state.h"
#include "system/system.h"
#include "e_key/RFID/RFID_handler.h"

static void system_sleepCheckWakeUp(void);

static uint8_t sleep_wakeUpPending = 0U;

void system_sleepInit(void){

	//speed mode output : OFF
	output_setState(&speedMode_High, GPIO_PIN_RESET);
	output_setState(&speedMode_Low, GPIO_PIN_RESET);
	output_setState(&speedMode_Reverse, GPIO_PIN_RESET);
//	output_setState(&speedMode_Cruise, GPIO_PIN_RESET);

	//relay : OFF
	output_setState(&ignition_rly, GPIO_PIN_RESET);
//	output_setState(&forceShutdown_rly, GPIO_PIN_RESET);


}

void system_sleepTask(void){
//	RFID_handleTask();
	system_sleepCheckWakeUp();
}

static void system_sleepCheckWakeUp(void){
	steeringLock_Status_t steeringStatus;
	/*
	 * logic state in this mode:
	 * SLEEP -> STANDBY
	 * SLEEP -> REGISTRATION
	 * SLEEP -> CHARGE
	 */

/* ========================================================= */
/* SLEEP -> STANDBY ======================================== */
/* ========================================================= */
	if (RFID_handleGetAuthEvent() != 0U){
		sleep_wakeUpPending = 1U;
		RFID_handleClearAuthEvent();
	}

	if (sleep_wakeUpPending != 0U){
		if (steeringLock_status == steeringLock_STATUS_MOVING){
			return;
		}

		if (steeringLock_status == steeringLock_STATUS_LOCK){
//			steeringStatus 		= steeringLock_Unlock();
			steeringStatus		= steeringLock_requestUnlock();
//			steeringLock_status	= steeringStatus;

			if (steeringStatus == steeringLock_STATUS_MOVING){
				return;
			}

			if (steeringStatus == steeringLock_STATUS_UNLOCK){
				sleep_wakeUpPending = 0U;
				system_setState(SYSTEM_STANDBY);
				authenticated = 0U;

				return;
			}

			return;
		}

		if (steeringLock_status == steeringLock_STATUS_UNLOCK){
			sleep_wakeUpPending = 0U;
			system_setState(SYSTEM_STANDBY);
			authenticated = 0U;

			return;
		}
	}

//	if (authenticated == 1){
//		steeringStatus = steeringLock_Unlock();
//
//		if (steeringStatus == steeringLock_STATUS_UNLOCK){
//			authenticated = 0;
//
//			system_setState(SYSTEM_STANDBY);
//		}
////		system_setState(SYSTEM_STANDBY);
////		authenticated = 0;
//		return;
//	}

/* ========================================================= */
/* SLEEP -> CHARGE ========================================= */
/* ========================================================= */
	if (charger == 1U) {
		system_setState(SYSTEM_CHARGE);
		return;
	}

/* ========================================================= */
/* SLEEP -> REGISTRATION =================================== */
/* ========================================================= */
//	else if () {
//		system_setState(SYSTEM_REGISTRATION);
//	}


}

