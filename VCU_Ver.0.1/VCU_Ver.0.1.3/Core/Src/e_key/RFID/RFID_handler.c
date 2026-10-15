/*
 * RFID_handler.c
 *
 *  Created on: Sep 30, 2026
 *      Author: fahmif_imoto
 */

#include "main.h"
#include "e_key/RFID/RFID_handler.h"

#include <string.h>
#include <stddef.h>

/* ========================================================= */
/* Master UID ============================================== */
/* ========================================================= */

const uint8_t RFID_masterUID[RFID_MASTER_UID_LENGTH] = {
//		0x63U, 0xD6U, 0xD5U, 0x38U
		0x11U, 0xD8U, 0xF7U, 0x64U
};

/* ========================================================= */
/* Debug Variable ========================================== */
/* ========================================================= */
volatile uint8_t RFID_handleAuthenticated 		= 0U;
volatile uint8_t RFID_handleUserID 				= RFID_INVALID_USER_ID;
volatile uint8_t RFID_handleCardPresent 		= 0U;
volatile uint8_t RFID_handleAuthEvent 			= 0U;
volatile uint8_t RFID_handleState 				= RFID_HANDLER_IDLE;

volatile uint8_t RFID_handleDetectConfirming	= 0U;
volatile uint32_t RFID_handleDetectElapsed		= 0U;

//nanti dihapus
volatile uint8_t RFID_handleReadStatus = 0U;
volatile uint8_t RFID_handleUID0 = 0U;
volatile uint8_t RFID_handleUID1 = 0U;
volatile uint8_t RFID_handleUID2 = 0U;
volatile uint8_t RFID_handleUID3 = 0U;
volatile uint8_t RFID_handleAuthStatus = 0U;

volatile uint8_t RFID_handleConfirmUID0 = 0U;
volatile uint8_t RFID_handleConfirmUID1 = 0U;
volatile uint8_t RFID_handleConfirmUID2 = 0U;
volatile uint8_t RFID_handleConfirmUID3 = 0U;

/* ========================================================= */
/* Internal Vatiable ======================================= */
/* ========================================================= */
static uint8_t RFID_handleLastUID[RFID_UID_LENGTH] 		= {0U}; 	// last UID that read to compare new card vs tapped card
static uint8_t RFID_handleLastUIDValid 					= 0U;		// 0 = no UID tapped, 1 = UID tapped
static uint8_t RFID_handleConfirmUID[RFID_UID_LENGTH]	= {0U};
static uint32_t RFID_handleDetectStartTick 				= 0U;		//

/* ========================================================= */
/* INTERNAL FUNCTION ======================================= */
/* ========================================================= */
static uint8_t RFID_handleCompare(const uint8_t *uid1, const uint8_t *uid2){
	if((uid1 == NULL) || (uid2 == NULL)){
		return 0U;
	}

	if(memcmp(uid1, uid2, RFID_UID_LENGTH) == 0){
		return 1U;
	}
	return 0U;
}

static void RFID_handleStartConfirmation(const uint8_t *uid){
	if (uid == NULL){
		return;
	}
	memcpy(RFID_handleConfirmUID, uid, RFID_UID_LENGTH);

    RFID_handleConfirmUID0 = uid[0];
    RFID_handleConfirmUID1 = uid[1];
    RFID_handleConfirmUID2 = uid[2];
    RFID_handleConfirmUID3 = uid[3];

	RFID_handleDetectStartTick 		= HAL_GetTick();
	RFID_handleDetectConfirming 	= 1U;
	RFID_handleDetectElapsed		= 0U;
}

static void RFID_handleCancelConfirmation(void){
	RFID_handleDetectConfirming		= 0U;
	RFID_handleDetectStartTick		= 0U;
	RFID_handleDetectElapsed		= 0U;

	memset(RFID_handleConfirmUID, 0, sizeof(RFID_handleConfirmUID));
}

static uint8_t RFID_handleConfirmDetection(const uint8_t *uid){
	uint32_t elapsed;
	if (uid == NULL){
		return 0U;
	}

	if (RFID_handleCompare(uid, RFID_handleConfirmUID) == 0){
		RFID_handleStartConfirmation(uid);

		return 0U;
	}

	elapsed = HAL_GetTick() - RFID_handleDetectStartTick;

	RFID_handleDetectElapsed = elapsed;

	if(elapsed < RFID_CONFIRM_TIME_MS){
		return 0U;
	}

	RFID_handleDetectConfirming = 0U;
	return 1U;
}

/* ========================================================= */
/* PUBLIC FUNCTION - Initialization ======================== */
/* ========================================================= */
void RFID_handleInit(void){
	RFID_managerInit();

	//handler status
	RFID_handleState 			= RFID_HANDLER_IDLE;
	RFID_handleAuthenticated 	= 0U;
	RFID_handleUserID 			= RFID_INVALID_USER_ID;
	RFID_handleCardPresent 		= 0U;
	RFID_handleAuthEvent     	= 0U;
	RFID_handleDetectConfirming	= 0U;
	RFID_handleDetectElapsed	= 0U;
	RFID_handleDetectStartTick	= 0U;

	// last UID
	memset(RFID_handleLastUID, 0, sizeof(RFID_handleLastUID));
	RFID_handleLastUIDValid 	= 0U;

	memset(RFID_handleConfirmUID, 0, sizeof(RFID_handleConfirmUID));

	RFID_SetMaster(RFID_masterUID);
}

/* ========================================================= */
/* PUBLIC FUNCTION - RFID Task ============================= */
/* ========================================================= */
void RFID_handleTask(void)
{
    uint8_t uid[RFID_UID_LENGTH] = {0U};
    uint8_t user_id = RFID_INVALID_USER_ID;
    RFID_ManagerStatus_t status;

    /* ===================================================== */
    /* Read RFID UID                                         */
    /* ===================================================== */

    status = RFID_readUID(uid);

    RFID_handleReadStatus = (uint8_t)status;

    RFID_handleUID0 = uid[0];
    RFID_handleUID1 = uid[1];
    RFID_handleUID2 = uid[2];
    RFID_handleUID3 = uid[3];

    /* ===================================================== */
    /* No RFID card/tag                                      */
    /* ===================================================== */

    if (status == RFID_MANAGER_NO_CARD)
    {
        RFID_handleCardPresent = 0U;
        RFID_handleAuthenticated = 0U;
        RFID_handleUserID = RFID_INVALID_USER_ID;
        RFID_handleAuthEvent = 0U;
        RFID_handleState = RFID_HANDLER_IDLE;

        RFID_handleLastUIDValid = 0U;
        memset(RFID_handleLastUID,
               0,
               sizeof(RFID_handleLastUID));

        RFID_handleCancelConfirmation();

        return;
    }

    /* ===================================================== */
    /* RFID error                                            */
    /* ===================================================== */

    if (status != RFID_MANAGER_OK)
    {
        RFID_handleState = RFID_HANDLER_ERROR;

        RFID_handleCancelConfirmation();

        return;
    }

    /* ===================================================== */
    /* Card detected                                         */
    /* ===================================================== */

    RFID_handleCardPresent = 1U;

    /* ===================================================== */
    /* Check whether this is the same authenticated card     */
    /* ===================================================== */

    if ((RFID_handleLastUIDValid != 0U) &&
        (RFID_handleCompare(uid, RFID_handleLastUID) != 0U))
    {
        if (RFID_handleAuthenticated != 0U)
        {
            RFID_handleState = RFID_HANDLER_CARD_HELD;
        }

        return;
    }

    /* ===================================================== */
    /* TEMPORARY: bypass 300 ms confirmation                  */
    /* ===================================================== */

    /*
     * Confirmation timer sementara di-bypass.
     *
     * Tujuan:
     * Menguji alur:
     *
     * RFID Read
     *     ->
     * RFID Authenticate
     *     ->
     * AuthEvent
     *     ->
     * SLEEP -> STANDBY
     */

    /* ===================================================== */
    /* Authentication                                        */
    /* ===================================================== */

    RFID_handleAuthEvent = 0U;

    RFID_handleAuthStatus = 0xAAU;

    status = RFID_Authenticate(uid, &user_id);

    RFID_handleAuthStatus = (uint8_t)status;

    /* ===================================================== */
    /* Authentication success                                */
    /* ===================================================== */

    if ((status == RFID_MANAGER_OK) ||
        (status == RFID_MANAGER_MASTER_UID))
    {
        memcpy(RFID_handleLastUID,
               uid,
               RFID_UID_LENGTH);

        RFID_handleLastUIDValid = 1U;

        RFID_handleAuthenticated = 1U;
        RFID_handleUserID = user_id;
        RFID_handleState = RFID_HANDLER_AUTHENTICATED;
        RFID_handleAuthEvent = 1U;

        return;
    }

    /* ===================================================== */
    /* Authentication failed                                 */
    /* ===================================================== */

    RFID_handleAuthenticated = 0U;
    RFID_handleUserID = RFID_INVALID_USER_ID;
    RFID_handleAuthEvent = 0U;
    RFID_handleState = RFID_HANDLER_NOT_AUTHENTICATED;
}


//void RFID_handleTask(void){
//	uint8_t uid[RFID_UID_LENGTH] = {0U};
//	uint8_t user_id = RFID_INVALID_USER_ID;
//	RFID_ManagerStatus_t status;
//
//	//Read RFID UID
//	status = RFID_readUID(uid);
//
//	RFID_handleReadStatus = (uint8_t)status;
//
//    RFID_handleUID0 = uid[0];
//    RFID_handleUID1 = uid[1];
//    RFID_handleUID2 = uid[2];
//    RFID_handleUID3 = uid[3];
//
//
//	//if no RFID card/tag
//	if(status == RFID_MANAGER_NO_CARD){
//		RFID_handleCardPresent 		= 0U;
//		RFID_handleAuthenticated 	= 0U;
//		RFID_handleUserID			= RFID_INVALID_USER_ID;
//		RFID_handleAuthEvent		= 0U;
//		RFID_handleState			= RFID_HANDLER_IDLE;
//
//		RFID_handleLastUIDValid		= 0U;
//		memset(RFID_handleLastUID, 0, sizeof(RFID_handleLastUID));
//		RFID_handleCancelConfirmation();
//
//		return;
//	}
//
//	//if RFID error
//	if (status != RFID_MANAGER_OK){
////		RFID_handleCardPresent 		= 0U;
////		RFID_handleAuthenticated 	= 0U;
////		RFID_handleUserID			= RFID_INVALID_USER_ID;
////		RFID_handleAuthEvent		= 0U;
//		RFID_handleState			= RFID_HANDLER_ERROR;
//
//		RFID_handleCancelConfirmation();
//
//		return;
//	}
//
//	//card detected
//	RFID_handleCardPresent = 1U;
//
//	//check same card
//	if ((RFID_handleLastUIDValid != 0U) && RFID_handleCompare(uid, RFID_handleLastUID) != 0U){
//		if (RFID_handleAuthenticated != 0U){
//			RFID_handleState = RFID_HANDLER_CARD_HELD;
//		}
//
//		return;
//	}
//
//	//New card detection
//	if (RFID_handleDetectConfirming == 0U){
//		RFID_handleStartConfirmation(uid);
//
//		return;
//	}
//
//	//Confirm UID stability
//	if (RFID_handleConfirmDetection(uid) == 0U){
//		return;
//	}
//
//	//save curent UID
////	memcpy(RFID_handleLastUID, uid, RFID_UID_LENGTH);
//	RFID_handleLastUIDValid = 1U;
//	RFID_handleAuthEvent = 0U;
//
//	//authenticate
//	RFID_handleAuthStatus = 0xAAU;
//	status = RFID_Authenticate(uid, &user_id);
//	RFID_handleAuthStatus = (uint8_t)status;
//
//	//if authentication success
//	if ((status == RFID_MANAGER_OK)||(status == RFID_MANAGER_MASTER_UID)){
//		memcpy(RFID_handleLastUID, uid, RFID_UID_LENGTH);
//		RFID_handleAuthenticated 	= 1U;
//		RFID_handleUserID 			= user_id;
//		RFID_handleState 			= RFID_HANDLER_AUTHENTICATED;
//		RFID_handleAuthEvent 		= 1U;
//
//		return;
//	}
//
//	//if authenticated failed
//	RFID_handleAuthenticated 		= 0U;
//	RFID_handleUserID 				= RFID_INVALID_USER_ID;
//	RFID_handleAuthEvent 			= 0U;
//	RFID_handleState 				= RFID_HANDLER_NOT_AUTHENTICATED;
//}



/* ========================================================= */
/* PUBLIC FUNCTION - Get Control State ===================== */
/* ========================================================= */
RFID_handleState_t RFID_handleGetState(void){
	return (RFID_handleState_t)RFID_handleState;
}

/* ========================================================= */
/* PUBLIC FUNCTION - Check Authentication ================== */
/* ========================================================= */
uint8_t RFID_handleIsAuthenticated(void){
	return RFID_handleAuthenticated;
}

/* ========================================================= */
/* PUBLIC FUNCTION - Get User ID =========================== */
/* ========================================================= */
uint8_t RFID_handleGetUserID(void){
	return RFID_handleUserID;
}

/* ========================================================= */
/* PUBLIC FUNCTION - Check Card Presence =================== */
/* ========================================================= */
uint8_t RFID_handleIsCardPresent(void){
	return RFID_handleCardPresent;
}

/* ========================================================= */
/* PUBLIC FUNCTION - Get Authenctication Event ============= */
/* ========================================================= */
uint8_t RFID_handleGetAuthEvent(void){
	return RFID_handleAuthEvent;
}

/* ========================================================= */
/* PUBLIC FUNCTION - Clear Authentication Event ============ */
/* ========================================================= */
void RFID_handleClearAuthEvent(void){
	RFID_handleAuthEvent = 0U;

	if ((RFID_handleCardPresent != 0U) && (RFID_handleAuthenticated != 0U)){
		RFID_handleState = RFID_HANDLER_CARD_HELD;
	}
}

/* ========================================================= */
/* PUBLIC FUNCTION - Clear Authentication State ============ */
/* ========================================================= */
void RFID_handleClearAuthentication(void){
	RFID_handleAuthenticated 	= 0U;
	RFID_handleUserID 			= RFID_INVALID_USER_ID;
	RFID_handleAuthEvent 		= 0U;

	if(RFID_handleCardPresent != 0U){
		RFID_handleState = RFID_HANDLER_CARD_HELD;
	}
	else {
		RFID_handleState = RFID_HANDLER_IDLE;
	}
//	RFID_handleState			= RFID_HANDLER_IDLE;
}

/* ========================================================= */
/* PUBLIC FUNCTION - Reset ================================= */
/* ========================================================= */
void RFID_handleReset(void){
	RFID_handleAuthenticated 	= 0U;
	RFID_handleUserID 			= RFID_INVALID_USER_ID;
	RFID_handleCardPresent 		= 0U;
	RFID_handleAuthEvent		= 0U;
	RFID_handleState 			= RFID_HANDLER_IDLE;
	RFID_handleLastUIDValid 	= 0U;

	memset(RFID_handleLastUID, 0, sizeof(RFID_handleLastUID));
	RFID_handleCancelConfirmation();
}
