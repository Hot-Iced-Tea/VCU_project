/*
 * RFID_handler.h
 *
 *  Created on: Sep 30, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_E_KEY_RFID_RFID_HANDLER_H_
#define INC_E_KEY_RFID_RFID_HANDLER_H_

#include <stdint.h>

#include "e_key/RFID/RFID_manager.h"

/* ========================================================= */
/* Configuration =========================================== */
/* ========================================================= */
#define RFID_MASTER_UID_LENGTH 	RFID_UID_LENGTH

#define RFID_CONFIRM_TIME_MS	300U

/* ========================================================= */
/* Read UID ================================================ */
/* ========================================================= */
extern const uint8_t RFID_masterUID[RFID_MASTER_UID_LENGTH];

/* ========================================================= */
/* Handler State =========================================== */
/* ========================================================= */
typedef enum{
    RFID_HANDLER_IDLE = 0U,				//handler IDLE
    RFID_HANDLER_AUTHENTICATED,			//handler authenticated
    RFID_HANDLER_NOT_AUTHENTICATED,		//handler not authenticated
    RFID_HANDLER_CARD_HELD,
	RFID_HANDLER_ERROR					//handler error
}RFID_handleState_t;

/* ========================================================= */
/* Debug Variable ========================================== */
/* ========================================================= */
extern volatile uint8_t RFID_handleAuthenticated;
extern volatile uint8_t RFID_handleUserID;
extern volatile uint8_t RFID_handleCardPresent;
extern volatile uint8_t RFID_handleAuthEvent;
extern volatile uint8_t RFID_handleState;

extern volatile uint8_t RFID_handleDetectConfirming;
extern volatile uint32_t RFID_handleDetectElapsed;

/* ========================================================= */
/* Public Function ========================================= */
/* ========================================================= */
void RFID_handleInit(void);						// initialization
void RFID_handleTask(void);						// main task

RFID_handleState_t RFID_handleGetState(void);	// get state
uint8_t RFID_handleIsAuthenticated(void);		// authentication check state
uint8_t RFID_handleGetUserID(void);				// get user ID
uint8_t RFID_handleIsCardPresent(void);			// present card read check state
uint8_t RFID_handleGetAuthEvent(void);			// get authentication event

void RFID_handleClearAuthEvent(void);			// clear authentication event
void RFID_handleClearAuthentication(void);		// clear authentication state
void RFID_handleReset(void);					// all reset

#endif /* INC_E_KEY_RFID_RFID_HANDLER_H_ */
