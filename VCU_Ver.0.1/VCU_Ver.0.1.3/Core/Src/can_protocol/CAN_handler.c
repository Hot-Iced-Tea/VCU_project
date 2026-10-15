/*
 * CAN_handler.c
 *
 *  Created on: Sep 16, 2026
 *      Author: fahmif_imoto
 */

#include "can_protocol/CAN_handler.h"
#include "cfg/CAN_config.h"

extern FDCAN_HandleTypeDef hfdcan1;

/* ============================================================ */
/* CAN RECEIVE DEBUG VARIABLES ================================ */
/* ============================================================ */

volatile uint32_t 	CAN_RxCount 		= 0U;	//Total number of CAN frames received
volatile uint32_t 	CAN_RxIdentifier 	= 0U; 	//Last received CAN Identifier
volatile uint32_t 	CAN_RxIdType 		= 0U;	//Last received CAN ID Type
volatile uint32_t 	CAN_RxFrameType 	= 0U; 	//Last received CAN Frame Type
volatile uint32_t 	CAN_RxDLC 			= 0U;	//Last received CAN DLC
volatile uint8_t 	CAN_RxData[8] = {			//Last CAN data received
	0xFFU,
	0xFFU,
	0xFFU,
	0xFFU,
	0xFFU,
	0xFFU,
	0xFFU,
	0xFFU
};
volatile uint32_t CAN_RxErrorCounter = 0U;

/* ============================================================ */
/* CAN TRANSMIT DEBUG VARIABLES =============================== */
/* ============================================================ */

volatile uint32_t 	CAN_TxCount 			= 0U;
volatile uint32_t 	CAN_TxErrorCount 		= 0U;
volatile uint32_t	CAN_TxBusyCount 		= 0U;
volatile uint32_t 	CAN_TxOtherErrorCount	= 0U;
volatile uint32_t 	CAN_TxLastStatus 		= 0U;
volatile uint32_t 	CAN_TxLastErrorCode 	= 0U;
volatile uint32_t 	CAN_TxFifoFreeLevel 	= 0U;
volatile uint32_t 	CAN_TxErrorCounter		= 0U;
volatile uint32_t CAN_TxCompleteCount 		= 0U;
volatile uint32_t CAN_TxCompleteIdentifier 	= 0U;

volatile uint32_t CAN_ErrorCode = 0U;

volatile uint32_t CAN_ProtocolLastErrorCode = 0U;
volatile uint32_t CAN_ProtocolDataLastErrorCode = 0U;
volatile uint32_t CAN_ProtocolActivity = 0U;
volatile uint32_t CAN_ProtocolErrorPassive = 0U;
volatile uint32_t CAN_ProtocolWarning = 0U;
volatile uint32_t CAN_ProtocolBusOff = 0U;
volatile uint32_t CAN_ProtocolRxESIflag = 0U;
volatile uint32_t CAN_ProtocolRxBRSflag = 0U;
volatile uint32_t CAN_ProtocolRxFDFflag = 0U;
volatile uint32_t CAN_ProtocolException = 0U;
volatile uint32_t CAN_ProtocolTDCvalue = 0U;


/* ============================================================
 * RX TX HEADER
 * ============================================================ */

static FDCAN_RxHeaderTypeDef CAN_RxHeader;
static FDCAN_TxHeaderTypeDef CAN_TxHeader;

/* ============================================================
 * INITIALIZATION
 * ============================================================ */

void CAN_Handler_Init(void)
{
    /*
     * Configure CAN:
     *
     * - Filter
     * - Global filter
     * - Start FDCAN
     * - Enable RX interrupt
     */
    CAN_config_Init();


    /* Reset RX debug variables */

    CAN_RxCount 		= 0U;
    CAN_RxIdentifier 	= 0U;
    CAN_RxIdType 		= 0U;
    CAN_RxFrameType 	= 0U;
    CAN_RxDLC 			= 0U;

    for (uint8_t i = 0U; i < 8U; i++)
    {
        CAN_RxData[i] = 0xFFU;
    }

    /* Reset TX debug variables */
    CAN_TxCount				= 0U;
    CAN_TxErrorCount	 	= 0U;
    CAN_TxBusyCount 		= 0U;
    CAN_TxOtherErrorCount 	= 0U;
}

HAL_StatusTypeDef CAN_Handler_Send(
    uint32_t id,
    uint32_t idType,
    const uint8_t *data,
    uint8_t dataLen){
	if (data == NULL){
		CAN_TxErrorCount++;
		return HAL_ERROR;
	}

	if (dataLen > 8U){
		CAN_TxErrorCount++;
		return HAL_ERROR;
	}

	/* Configure CAN frame*/
	CAN_TxHeader.Identifier 	= id;
	CAN_TxHeader.IdType 		= idType;
	CAN_TxHeader.TxFrameType 	= FDCAN_DATA_FRAME;

	/*
     * Convert payload length into
     * STM32 FDCAN DLC format.
     */
	switch (dataLen){
	case 0U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_0;
	break;

	case 1U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_1;
	break;

	case 2U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_2;
	break;

	case 3U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_3;
	break;

	case 4U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_4;
	break;

	case 5U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_5;
	break;

	case 6U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_6;
	break;

	case 7U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_7;
	break;

	case 8U:
		CAN_TxHeader.DataLength = FDCAN_DLC_BYTES_8;
	break;

	default:
		CAN_TxErrorCount++;
		return HAL_ERROR;
	}


	//Classic CAN configuration
	CAN_TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	CAN_TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	CAN_TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	CAN_TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	CAN_TxHeader.MessageMarker = 0U;


	//Send message
	HAL_StatusTypeDef status;

	status = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &CAN_TxHeader, (uint8_t *)data);

	CAN_TxLastStatus = (uint32_t)status;

	if (status != HAL_OK){
		CAN_TxErrorCount++;
		CAN_TxLastErrorCode = HAL_FDCAN_GetError(&hfdcan1);

		if(status == HAL_BUSY){
			CAN_TxBusyCount++;
		}
		else {
			CAN_TxOtherErrorCount++;
		}

		return HAL_ERROR;
	}

	// TX successful
	CAN_TxCount++;
	return HAL_OK;

}

void CAN_Handler_UpdateDebug(void)
{
    FDCAN_ErrorCountersTypeDef errorCounters;
    FDCAN_ProtocolStatusTypeDef protocolStatus;

    CAN_TxFifoFreeLevel =
        HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1);

    CAN_ErrorCode =
        HAL_FDCAN_GetError(&hfdcan1);

    if (HAL_FDCAN_GetErrorCounters(
            &hfdcan1,
            &errorCounters) == HAL_OK)
    {
        CAN_TxErrorCounter = errorCounters.TxErrorCnt;
        CAN_RxErrorCounter = errorCounters.RxErrorCnt;
    }

    if (HAL_FDCAN_GetProtocolStatus(
            &hfdcan1,
            &protocolStatus) == HAL_OK)
    {
        CAN_ProtocolLastErrorCode =
            protocolStatus.LastErrorCode;

        CAN_ProtocolDataLastErrorCode =
            protocolStatus.DataLastErrorCode;

        CAN_ProtocolActivity =
            protocolStatus.Activity;

        CAN_ProtocolErrorPassive =
            protocolStatus.ErrorPassive;

        CAN_ProtocolWarning =
            protocolStatus.Warning;

        CAN_ProtocolBusOff =
            protocolStatus.BusOff;

        CAN_ProtocolRxESIflag =
            protocolStatus.RxESIflag;

        CAN_ProtocolRxBRSflag =
            protocolStatus.RxBRSflag;

        CAN_ProtocolRxFDFflag =
            protocolStatus.RxFDFflag;

        CAN_ProtocolException =
            protocolStatus.ProtocolException;

        CAN_ProtocolTDCvalue =
            protocolStatus.TDCvalue;
    }
}

/* ============================================================
 * CAN RX FIFO0 CALLBACK
 * ============================================================ */

void HAL_FDCAN_RxFifo0Callback(
    FDCAN_HandleTypeDef *hfdcan,
    uint32_t RxFifo0ITs)
{
	//Make sure the interrupt belongs to FDCAN1.
    if (hfdcan->Instance != FDCAN1)
    {
        return;
    }


    /*
     * Check RX FIFO0 New Message interrupt.
     */

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0U)
    {
        return;
    }


    /*
     * Read the CAN frame from RX FIFO0.
     */

    if (HAL_FDCAN_GetRxMessage(
            hfdcan,
            FDCAN_RX_FIFO0,
            &CAN_RxHeader,
            (uint8_t *)CAN_RxData) != HAL_OK)
    {
        return;
    }


    /* ========================================================
     * SAVE LAST RECEIVED FRAME
     * ======================================================== */

    CAN_RxIdentifier 	= CAN_RxHeader.Identifier;
    CAN_RxIdType 		= CAN_RxHeader.IdType;
    CAN_RxFrameType 	= CAN_RxHeader.RxFrameType;
    CAN_RxDLC 			= CAN_RxHeader.DataLength;


    /*
     * One new frame has been received.
     */

    CAN_RxCount++;
}
