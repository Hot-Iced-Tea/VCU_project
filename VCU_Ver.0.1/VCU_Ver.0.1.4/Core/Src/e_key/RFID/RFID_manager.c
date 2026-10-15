/*
 * RFID_manager.c
 *
 *  Created on: Sep 29, 2026
 *      Author: fahmif_imoto
 */

#include "e_key/RFID/RFID_manager.h"
#include "e_key/RFID/driver/RC522.h"

#include <string.h>

/* ========================================================= */
/* DEBUG VARIABLE ========================================== */
/* ========================================================= */
volatile uint8_t RFID_RequestStatus = 0;
volatile uint8_t RFID_AnticollStatus = 0;
volatile uint8_t RFID_UID[RFID_UID_LENGTH] = {0};
volatile uint8_t RFID_UID_Valid = 0;
volatile uint8_t RFID_Detected = 0;

/* ========================================================= */
/* INTERNAL DATABASE ======================================= */
/* ========================================================= */
static RFID_User_t RFID_UserDatabase[RFID_MAX_USERS];


/*
 * Master RFID
 *
 * valid = 0 -> master belum diset
 * valid = 1 -> master sudah diset
 */
static struct{
    uint8_t valid;
    uint8_t uid[RFID_UID_LENGTH];
} RFID_Master;

/* ========================================================= */
/* PRIVATE FUNCTION ======================================== */
/* ========================================================= */
static uint8_t RFID_UIDCompare(const uint8_t *uid1, const uint8_t *uid2){
    if ((uid1 == NULL) || (uid2 == NULL)){
        return 0;
    }

    return (memcmp(uid1, uid2, RFID_UID_LENGTH) == 0);
}

static int RFID_FindUserByUID(const uint8_t *uid){
    uint8_t i;

    if (uid == NULL){
        return -1;
    }

    for (i = 0; i < RFID_MAX_USERS; i++){
        if (RFID_UserDatabase[i].valid == 1U){
            if (RFID_UIDCompare(RFID_UserDatabase[i].uid,uid)){
                return i;
            }
        }
    }

    return -1;
}


static int RFID_FindUserByID(uint8_t user_id){
    uint8_t i;

    for (i = 0; i < RFID_MAX_USERS; i++){
        if (RFID_UserDatabase[i].valid == 1U){
            if (RFID_UserDatabase[i].user_id == user_id){
                return i;
            }
        }
    }

    return -1;
}


static int RFID_FindFreeSlot(void){
    uint8_t i;

    for (i = 0; i < RFID_MAX_USERS; i++){
        if (RFID_UserDatabase[i].valid == 0U){
            return i;
        }
    }

    return -1;
}


/* ========================================================= */
/* Initialization ========================================== */
/* ========================================================= */

void RFID_managerInit(void){
    memset(RFID_UserDatabase, 0, sizeof(RFID_UserDatabase));
    memset(&RFID_Master, 0, sizeof(RFID_Master));

    RFID_RequestStatus = 0;
    RFID_AnticollStatus = 0;

    memset((void *)RFID_UID, 0, sizeof(RFID_UID));

    RFID_UID_Valid = 0;
    RFID_Detected = 0;

    /*
     * Initialize RC522 driver
     */
    MFRC522_Init();
}


/* ========================================================= */
/* Read UID ================================================ */
/* ========================================================= */

RFID_ManagerStatus_t RFID_readUID(uint8_t *uid){
    uint8_t str[MAX_LEN];

    if (uid == NULL){
        return RFID_MANAGER_INVALID_UID;
    }

    RFID_Detected = 0;
    RFID_UID_Valid = 0;

    //Request RFID card
    RFID_RequestStatus = MFRC522_Request(PICC_REQIDL, str);

    if (RFID_RequestStatus != MI_OK){
        return RFID_MANAGER_NO_CARD;
    }

    RFID_Detected = 1;


    //Anti-collision
    RFID_AnticollStatus = MFRC522_Anticoll(str);
    if (RFID_AnticollStatus != MI_OK){
        return RFID_MANAGER_ANTICOLL_ERROR;
    }

    //Copy UID
    memcpy(uid, str, RFID_UID_LENGTH);


    //Copy to debug variable
    memcpy((void *)RFID_UID, str, RFID_UID_LENGTH);

    RFID_UID_Valid = 1;

    return RFID_MANAGER_OK;
}

/* ========================================================= */
/* RFID Detection ========================================== */
/* ========================================================= */
uint8_t RFID_isDetected(void){
    return RFID_Detected;
}

/* ========================================================= */
/* RFID Clear ============================================== */
/* ========================================================= */
void RFID_ClearUID(void){
    memset((void *)RFID_UID, 0, sizeof(RFID_UID));
    RFID_UID_Valid 			= 0;
    RFID_Detected 			= 0;
    RFID_RequestStatus 		= 0;
    RFID_AnticollStatus 	= 0;
}

/* ========================================================= */
/* RFID Register =========================================== */
/* ========================================================= */
RFID_ManagerStatus_t RFID_Register(uint8_t user_id, const uint8_t *uid){
    int existing_slot;
    int free_slot;

    if (uid == NULL){
        return RFID_MANAGER_INVALID_UID;
    }

    //User ID tidak boleh menggunakan
    //ID invalid.
    if (user_id == RFID_INVALID_USER_ID){
        return RFID_MANAGER_INVALID_UID;
    }

    /*
     * Jangan register Master RFID
     * sebagai user biasa.
     */
    if (RFID_IsMaster(uid)){
        return RFID_MANAGER_MASTER_UID;
    }

    //check wether UID registered?
    existing_slot = RFID_FindUserByUID(uid);

    if (existing_slot >= 0){
        return RFID_MANAGER_ALREADY_REGISTERED;
    }

    //Check wether User ID used?
    existing_slot = RFID_FindUserByID(user_id);

    if (existing_slot >= 0){
        return RFID_MANAGER_ALREADY_REGISTERED;
    }

    //Searche empty slot
    free_slot = RFID_FindFreeSlot();

    if (free_slot < 0){
        return RFID_MANAGER_DATABASE_FULL;
    }

    //Save user
    RFID_UserDatabase[free_slot].valid = 1U;

    RFID_UserDatabase[free_slot].user_id = user_id;

    memcpy(RFID_UserDatabase[free_slot].uid, uid, RFID_UID_LENGTH);

    return RFID_MANAGER_OK;
}


/* ========================================================= */
/* AUTHENTICATE ============================================ */
/* ========================================================= */
RFID_ManagerStatus_t RFID_Authenticate(const uint8_t *uid, uint8_t *user_id){
    int slot;

    if ((uid == NULL) || (user_id == NULL)){
        return RFID_MANAGER_INVALID_UID;
    }

    /*
     * Master card juga merupakan
     * credential yang valid.
     */
    if (RFID_IsMaster(uid)){
        *user_id = 0U;

        return RFID_MANAGER_MASTER_UID;
    }

    /*
     * Cari UID di database
     */
    slot = RFID_FindUserByUID(uid);

    if (slot < 0){
        *user_id = RFID_INVALID_USER_ID;

        return RFID_MANAGER_NOT_REGISTERED;
    }

    /*
     * UID ditemukan
     */
    *user_id = RFID_UserDatabase[slot].user_id;

    return RFID_MANAGER_OK;
}


/* ========================================================= */
/* RFID Delete ============================================= */
/* ========================================================= */

RFID_ManagerStatus_t RFID_Delete(uint8_t user_id)
{
    int slot;

    /*
     * Jangan izinkan Master ID
     * dihapus melalui fungsi user biasa.
     */
    if (user_id == 0U){
        return RFID_MANAGER_MASTER_PROTECTED;
    }

    slot = RFID_FindUserByID(user_id);

    if (slot < 0){
        return RFID_MANAGER_NOT_REGISTERED;
    }

    /*
     * Clear record
     */
    memset(&RFID_UserDatabase[slot], 0, sizeof(RFID_User_t));

    return RFID_MANAGER_OK;
}


/* ========================================================= */
/* Is Registered? ========================================== */
/* ========================================================= */

uint8_t RFID_IsRegistered(const uint8_t *uid){
    if (uid == NULL){
        return 0;
    }

    /*
     * Master dianggap credential
     * yang terdaftar secara khusus.
     */
    if (RFID_IsMaster(uid)){
        return 1U;
    }

    if (RFID_FindUserByUID(uid) >= 0){
        return 1U;
    }

    return 0U;
}

/* ========================================================= */
/* Set Master ============================================== */
/* ========================================================= */
RFID_ManagerStatus_t RFID_SetMaster(const uint8_t *uid){
    if (uid == NULL){
        return RFID_MANAGER_INVALID_UID;
    }

    /*
     * Untuk keamanan, master hanya boleh
     * ditetapkan jika belum ada.
     */
    if (RFID_Master.valid == 1U){
//        return RFID_MANAGER_MASTER_PROTECTED;
        return RFID_MANAGER_ALREADY_REGISTERED;
    }

    memcpy(RFID_Master.uid, uid, RFID_UID_LENGTH);

    RFID_Master.valid = 1U;

    return RFID_MANAGER_OK;
}

/* ========================================================= */
/* Is Master? ============================================== */
/* ========================================================= */
uint8_t RFID_IsMaster(const uint8_t *uid){
    if (uid == NULL){
        return 0;
    }

    if (RFID_Master.valid == 0U){
        return 0;
    }

    return RFID_UIDCompare(RFID_Master.uid, uid);
}


/* =========================================================
 * Is Master Set
 * ========================================================= */

uint8_t RFID_IsMasterSet(void){
    return RFID_Master.valid;
}
