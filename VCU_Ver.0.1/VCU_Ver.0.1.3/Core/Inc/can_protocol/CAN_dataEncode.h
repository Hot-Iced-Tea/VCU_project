/*
 * CAN_dataEncode.h
 *
 *  Created on: Sep 24, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_CAN_PROTOCOL_CAN_DATAENCODE_H_
#define INC_CAN_PROTOCOL_CAN_DATAENCODE_H_

#include "main.h"

//CAN Payload
extern uint8_t VCU_StatusPackage1[8];
extern uint8_t VCU_StatusPackage2[8];

extern uint8_t VCU_Request1[8];
extern uint8_t VCU_Request2[8];
extern uint8_t VCU_Request3[8];
extern uint8_t VCU_Request4[8];

extern uint8_t VCU_displaySetting1[8];
extern uint8_t VCU_displaySetting2[8];

extern uint8_t VCU_vehiclePosition[8];

extern uint8_t VCU_toBMScmd[8];

//CAN Encoder Process
void CAN_dataEncode_init(void);

void CAN_Encode_VCU_StatusPackage1(void);
void CAN_Encode_VCU_StatusPackage2(void);

void CAN_Encode_VCU_Request1(void);
void CAN_Encode_VCU_Request2(void);
void CAN_Encode_VCU_Request3(void);
void CAN_Encode_VCU_Request4(void);

void CAN_Encode_VCU_displaySetting1(void);
void CAN_Encode_VCU_displaySetting2(void);

void CAN_Encode_VCU_vehiclePosition(void);

void CAN_Encode_VCU_toBMScmd(void);

#endif /* INC_CAN_PROTOCOL_CAN_DATAENCODE_H_ */
