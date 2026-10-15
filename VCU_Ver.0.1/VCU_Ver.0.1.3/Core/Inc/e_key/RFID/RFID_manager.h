/*
 * RFID_manager.h
 *
 *  Created on: Sep 29, 2026
 *      Author: fahmif_imoto
 */

#ifndef RFID_MANAGER_H
#define RFID_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ========================================================= */
/* Configuration =========================================== */
/* ========================================================= */
#define RFID_UID_LENGTH       4U
#define RFID_MAX_USERS        10U
#define RFID_INVALID_USER_ID  0xFFU

/* ========================================================= */
/* RFID Manager Status ===================================== */
/* ========================================================= */
typedef enum
{
    RFID_MANAGER_OK = 0,

    RFID_MANAGER_NO_CARD,
    RFID_MANAGER_REQUEST_ERROR,
    RFID_MANAGER_ANTICOLL_ERROR,
    RFID_MANAGER_INVALID_UID,

    RFID_MANAGER_ALREADY_REGISTERED,
    RFID_MANAGER_NOT_REGISTERED,
    RFID_MANAGER_DATABASE_FULL,

    RFID_MANAGER_MASTER_NOT_SET,
    RFID_MANAGER_MASTER_UID,
    RFID_MANAGER_MASTER_PROTECTED

} RFID_ManagerStatus_t;


/* ========================================================= */
/* RFID User Data ========================================== */
/* ========================================================= */
typedef struct{
    uint8_t valid;
    uint8_t user_id;
    uint8_t uid[RFID_UID_LENGTH];
} RFID_User_t;


/* ========================================================= */
/* Public Fucntion ========================================= */
/* ========================================================= */
void RFID_managerInit(void); 						// Initialize RFID manager
RFID_ManagerStatus_t RFID_readUID(uint8_t *uid); 	// Read UID from RC522
uint8_t RFID_isDetected(void); 						// Check whether card is detected
void RFID_ClearUID(void);							// Clear current UID


/* ========================================================= */
/* User Management ========================================= */
/* ========================================================= */
RFID_ManagerStatus_t RFID_Register(uint8_t user_id, const uint8_t *uid); 		//Register a new RFID user
RFID_ManagerStatus_t RFID_Authenticate(const uint8_t *uid, uint8_t *user_id); 	//Authenticate RFID UID
RFID_ManagerStatus_t RFID_Delete(uint8_t user_id); 								//Delete registered RFID user
uint8_t RFID_IsRegistered(const uint8_t *uid);									//Check whether UID is registered


/* ========================================================= */
/* MASTER RFID ============================================= */
/* ========================================================= */
RFID_ManagerStatus_t RFID_SetMaster(const uint8_t *uid);	// Set currently supplied UID as master
uint8_t RFID_IsMaster(const uint8_t *uid);					// Check whether UID is master
uint8_t RFID_IsMasterSet(void);								// Check whether master has been configured


/* ========================================================= */
/* Debug Variable ========================================== */
/* ========================================================= */

extern volatile uint8_t RFID_RequestStatus;
extern volatile uint8_t RFID_AnticollStatus;

extern volatile uint8_t RFID_UID[RFID_UID_LENGTH];

extern volatile uint8_t RFID_UID_Valid;
extern volatile uint8_t RFID_Detected;


/* ========================================================= */

#ifdef __cplusplus
}
#endif

#endif /* RFID_MANAGER_H */
