/*
 * CAN_dataBroadcast.h
 *
 *  Created on: Sep 17, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_CAN_PROTOCOL_CAN_DATABROADCAST_H_
#define INC_CAN_PROTOCOL_CAN_DATABROADCAST_H_

#include "main.h"

#define CAN_VCUstatusPackage1		0x1800FCA1U
#define CAN_VCUstatusPackage2		0x1801FCA1U
#define CAN_VCUreqest1				0x1802FCA1U
#define CAN_VCUreqest2				0x1803FCA1U
#define CAN_VCUreqest3				0x1804FCA1U
#define CAN_VCUreqest4				0x1805FCA1U
#define CAN_VCUdisplaySetting1		0x1806FCA1U
#define CAN_VCUdisplaySetting2		0x1807FCA1U
#define CAN_VCUvehiclePosition		0x1808FCA1U
#define CAN_VCUtoBMScmd				0x181250F4U


void CAN_VCUbroadcast_Init(void);
void CAN_VCUbroadcast_Task(void);

#endif /* INC_CAN_PROTOCOL_CAN_DATABROADCAST_H_ */
