/*
 * CAN_dataBroadcast.c
 *
 *  Created on: Sep 17, 2026
 *      Author: fahmif_imoto
 */

#include "can_protocol/CAN_dataBroadcast.h"
#include "can_protocol/CAN_handler.h"
#include "can_protocol/CAN_dataEncode.h"

#define CAN_BROADCAST_PERIODE_MS	100U
#define CAN_BROADCAST_BATCH_COUNT	5U

static uint8_t canBroadcastBatch = 0U;
static uint32_t canBroadcastLastTick = 0U;

static void CAN_VCUbroadcastBatch(uint8_t batch){
	switch (batch){
	case 0U:
		CAN_Encode_VCU_StatusPackage1();
		CAN_Handler_Send(CAN_VCUstatusPackage1, FDCAN_EXTENDED_ID, VCU_StatusPackage1, 8U);

		CAN_Encode_VCU_StatusPackage2();
		CAN_Handler_Send(CAN_VCUstatusPackage2, FDCAN_EXTENDED_ID, VCU_StatusPackage2, 8U);
		break;

	case 1U:
		CAN_Encode_VCU_Request1();
		CAN_Handler_Send(CAN_VCUreqest1, FDCAN_EXTENDED_ID, VCU_Request1, 8U);

		CAN_Encode_VCU_Request2();
		CAN_Handler_Send(CAN_VCUreqest2, FDCAN_EXTENDED_ID, VCU_Request2, 8U);
		break;

	case 2U:
		CAN_Encode_VCU_Request3();
		CAN_Handler_Send(CAN_VCUreqest3, FDCAN_EXTENDED_ID, VCU_Request3, 8U);

		CAN_Encode_VCU_Request4();
		CAN_Handler_Send(CAN_VCUreqest4, FDCAN_EXTENDED_ID, VCU_Request4, 8U);
		break;

	case 3U:
		CAN_Encode_VCU_displaySetting1();
		CAN_Handler_Send(CAN_VCUdisplaySetting1, FDCAN_EXTENDED_ID, VCU_displaySetting1, 8U);

		CAN_Encode_VCU_displaySetting2();
		CAN_Handler_Send(CAN_VCUdisplaySetting2, FDCAN_EXTENDED_ID, VCU_displaySetting2, 8U);
		break;

	case 4U:
		CAN_Encode_VCU_vehiclePosition();
		CAN_Handler_Send(CAN_VCUvehiclePosition, FDCAN_EXTENDED_ID, VCU_vehiclePosition, 8U);

		CAN_Encode_VCU_toBMScmd();
		CAN_Handler_Send(CAN_VCUtoBMScmd, FDCAN_EXTENDED_ID, VCU_toBMScmd, 8U);
		break;

	default:
		canBroadcastBatch = 0U;
		break;

	}
}

void CAN_VCUbroadcast_Init(void)
{
    canBroadcastBatch =0U;
    canBroadcastLastTick = HAL_GetTick();
}

void CAN_VCUbroadcast_Task(void)
{
    uint32_t currentTick = HAL_GetTick();
    if ((currentTick - canBroadcastLastTick) < CAN_BROADCAST_PERIODE_MS){
    	return;
    }

    canBroadcastLastTick = currentTick;
    CAN_VCUbroadcastBatch(canBroadcastBatch);
    canBroadcastBatch++;

    if (canBroadcastBatch >= CAN_BROADCAST_BATCH_COUNT){
    	canBroadcastBatch = 0U;
    }
}
