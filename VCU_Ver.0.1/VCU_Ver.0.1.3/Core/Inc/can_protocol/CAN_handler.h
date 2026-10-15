/*
 * CAN_handler.h
 *
 *  Created on: Sep 16, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_CAN_PROTOCOL_CAN_HANDLER_H_
#define INC_CAN_PROTOCOL_CAN_HANDLER_H_

#include "main.h"
#include "stm32h5xx_hal.h"


/* ============================================================ */
/* CAN RECEIVE DEBUG VARIABLES ================================ */
/* ============================================================ */

extern volatile uint32_t 	CAN_RxCount; 		//Total number of CAN frames received
extern volatile uint32_t 	CAN_RxIdentifier; 	//Last received CAN Identifier
extern volatile uint32_t 	CAN_RxIdType; 		//Last received CAN ID Type
extern volatile uint32_t 	CAN_RxFrameType; 	//Last received CAN Frame Type
extern volatile uint32_t 	CAN_RxDLC;			//Last received CAN DLC
extern volatile uint8_t 	CAN_RxData[8]; 		//Last CAN data received
extern volatile uint32_t CAN_RxErrorCounter;

/* ============================================================ */
/* CAN TRANSMIT DEBUG VARIABLES =============================== */
/* ============================================================ */

extern volatile uint32_t 	CAN_TxCount;
extern volatile uint32_t 	CAN_TxErrorCount;
extern volatile uint32_t 	CAN_TxBusyCount;
extern volatile uint32_t 	CAN_TxOtherErrorCount;
extern volatile uint32_t 	CAN_TxLastStatus;
extern volatile uint32_t 	CAN_TxLastErrorCode;
extern volatile uint32_t 	CAN_TxFifoFreeLevel;
extern volatile uint32_t	CAN_TxErrorCounter;
extern volatile uint32_t CAN_TxCompleteCount;
extern volatile uint32_t CAN_TxCompleteIdentifier;

extern volatile uint32_t CAN_ErrorCode;

extern volatile uint32_t CAN_ProtocolLastErrorCode;
extern volatile uint32_t CAN_ProtocolDataLastErrorCode;
extern volatile uint32_t CAN_ProtocolActivity;
extern volatile uint32_t CAN_ProtocolErrorPassive;
extern volatile uint32_t CAN_ProtocolWarning;
extern volatile uint32_t CAN_ProtocolBusOff;
extern volatile uint32_t CAN_ProtocolRxESIflag;
extern volatile uint32_t CAN_ProtocolRxBRSflag;
extern volatile uint32_t CAN_ProtocolRxFDFflag;
extern volatile uint32_t CAN_ProtocolException;
extern volatile uint32_t CAN_ProtocolTDCvalue;

/* ============================================================ */
/* CAN HANDLER ================================================ */
/* ============================================================ */

/*
 * Initialize CAN Handler.
 *
 * This function:
 * - Configures CAN filter
 * - Configures global filter
 * - Starts FDCAN
 * - Enables RX FIFO0 interrupt
 */
void CAN_Handler_Init(void);

/* ============================================================
 * CAN Transmit ===============================================
 * ============================================================
 *
 * id       : CAN identifier
 * idType   : FDCAN_STANDARD_ID / FDCAN_EXTENDED_ID
 * data     : pointer to payload
 * dataLen  : payload length, 0..8 bytes
 *
 * return   : HAL_OK / HAL_ERROR
 */

HAL_StatusTypeDef CAN_Handler_Send(
    uint32_t id,
    uint32_t idType,
    const uint8_t *data,
    uint8_t dataLen
);

void CAN_Handler_UpdateDebug(void);


#endif /* INC_CAN_PROTOCOL_CAN_HANDLER_H_ */


