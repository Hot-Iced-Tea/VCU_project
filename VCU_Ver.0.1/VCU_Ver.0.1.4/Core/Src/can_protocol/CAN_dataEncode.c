/*
 * CAN_dataEncode.c
 *
 *  Created on: Sep 24, 2026
 *      Author: fahmif_imoto
 */


#include "can_protocol/CAN_dataEncode.h"
//GPIO
#include "cfg/GPIO_config.h"
#include "IO_control/input_ctrl.h"
#include "IO_control/output_ctrl.h"
#include "IO_control/steeringLock_control.h"
//system
#include "system/system.h"
#include "system/modes_state/ride_mode.h"

// CAN_GPIO
#define CAN_steeringLock_sw_BIT   		(1U << 0)
#define CAN_sideStand_sw_BIT       		(1U << 1)
#define CAN_speedMode_sw_BIT    	   	(1U << 2)
#define CAN_seatLock_sw_BIT 	       	(1U << 3)
#define CAN_forceShutdown_status_BIT   	(1U << 4)
#define CAN_ignition_status_BIT        (1U << 5)

#define CAN_ignition_rly_BIT			(1U << 0)
#define CAN_horn_BIT					(1U << 1)
#define CAN_longBeam_BIT				(1U << 2)
#define CAN_forceShutdown_rly_BIT		(1U << 3)
#define CAN_speedMode_High_BIT			(1U << 4)
#define CAN_speedMode_Low_BIT			(1U << 5)
#define CAN_speedMode_Reverse_BIT		(1U << 6)
#define CAN_speedMode_Cruise_BIT		(1U << 7)

//CAN Payload
uint8_t VCU_StatusPackage1[8] 	= {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
uint8_t VCU_StatusPackage2[8] 	= {0x00U, 0x01U, 0x10U, 0x11U, 0x00U, 0x00U, 0x00U, 0x00U};

uint8_t VCU_Request1[8]			= {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U};
uint8_t VCU_Request2[8]			= {0x00U, 0x10U, 0x20U, 0x30U, 0x40U, 0x50U, 0x60U, 0x70U};
uint8_t VCU_Request3[8]			= {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
uint8_t VCU_Request4[8]			= {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

uint8_t VCU_displaySetting1[8]	= {0x0AU, 0x0BU, 0x0CU, 0x0DU, 0x0EU, 0x0FU, 0xF0U, 0xFFU};
uint8_t VCU_displaySetting2[8]	= {0xA0U, 0xB0U, 0xC0U, 0xD0U, 0xE0U, 0xF0U, 0x0FU, 0x00U};

uint8_t VCU_vehiclePosition[8]	= {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

uint8_t VCU_toBMScmd[8]			= {0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

//CAN Init
void CAN_dataEncode_init(void){

}

void CAN_Encode_VCU_StatusPackage1(void){
	// Byte 0 data : State System
	systemState_t systemState;
	systemState = system_GetState();

	switch (systemState){
	case SYSTEM_SLEEP 		: VCU_StatusPackage1[0] = 0x00U;
		break;
	case SYSTEM_STANDBY		: VCU_StatusPackage1[0] = 0x11U;
		break;
	case SYSTEM_RIDE		: VCU_StatusPackage1[0] = 0x22U;
		break;
	case SYSTEM_REGISTRATION: VCU_StatusPackage1[0] = 0x44U;
		break;
	case SYSTEM_CHARGE		: VCU_StatusPackage1[0] = 0x88U;
		break;
	case SYSTEM_FAULT		: VCU_StatusPackage1[0] = 0xFFU;
		break;
	default:
		VCU_StatusPackage1[0] = 0x00U;
		break;

	}

	// Byte 1 data : Sub-state System
	switch (systemState){
	case SYSTEM_SLEEP		:
		VCU_StatusPackage1[1] = 0x00U;
		break;
	case SYSTEM_STANDBY :
		//substate Standby list
		VCU_StatusPackage1[1] = 0x1FU;
		break;
	case SYSTEM_RIDE	:
		//ride mode list
		rideMode_t rideMode;
		rideMode = rideMode_Get();
		switch (rideMode){
		case RideMode_ECO		: VCU_StatusPackage1[1] = 0x20U;
			break;
		case RideMode_DRIVE		: VCU_StatusPackage1[1] = 0x21U;
			break;
		case RideMode_SPORT		: VCU_StatusPackage1[1] = 0x22U;
			break;
		case RideMode_REVERSE	: VCU_StatusPackage1[1] = 0x23U;
			break;
		case RideMode_CRUISE	: VCU_StatusPackage1[1] = 0x24U;
			break;
		default:
			VCU_StatusPackage1[1] = 0x20U; //ECO mode
			break;
		}
		break;

	case SYSTEM_REGISTRATION:
		//registration list
		VCU_StatusPackage1[1] = 0x4FU;
		break;
	case SYSTEM_CHARGE:
		//charge list
		VCU_StatusPackage1[1] = 0x8FU;
		break;
	case SYSTEM_FAULT:
		//fault code
		VCU_StatusPackage1[1] = 0xFFU;
		break;
	default:
		VCU_StatusPackage1[1] = 0x00U;
		break;
	}
	// Byte 2 data : Vehicel Fault ID
	// Byte 3 data : Vehicel Fault Code
	// Byte 4 data : Vehicel Fault Level
	// Byte 5 data : VCU Reserved 0
	// Byte 6 data : VCU Reserved 1
	// Byte 7 data : VCU Reserved 2
}

void CAN_Encode_VCU_StatusPackage2(void){
	// Byte 0 data : VCU_Working_Status
	// Byte 1 data : BMS_Working_Status
	// Byte 2 data : Display_Working_Status
	// Byte 3 data : MCU_Working_Status

	// Byte 4 data : VCU Digital Input Status
	VCU_StatusPackage2[4] = 0U;
	if (input_getState(&steeringLock_sw) == GPIO_PIN_SET) 		VCU_StatusPackage2[4] |= CAN_steeringLock_sw_BIT;
	if (input_getState(&sideStand_sw) == GPIO_PIN_SET)			VCU_StatusPackage2[4] |= CAN_sideStand_sw_BIT;
	if (input_getState(&speedMode_sw) == GPIO_PIN_SET)			VCU_StatusPackage2[4] |= CAN_speedMode_sw_BIT;
	if (input_getState(&seatLock_sw) == GPIO_PIN_SET) 			VCU_StatusPackage2[4] |= CAN_seatLock_sw_BIT;
//	if (input_getState(&ignition_state) == GPIO_PIN_SET)		VCU_StatusPackage2[4] |= CAN_ignition_status_BIT;
//	if (input_getState(&forceShutdown_state) == GPIO_PIN_SET)VCU_StatusPackage2[4] |= CAN_forceShutdown_status_BIT;

	// Byte 5 data : VCU Digital Output Status
	VCU_StatusPackage2[5] = 0U;
	if (output_getState(&ignition_rly) 		== GPIO_PIN_SET) 	VCU_StatusPackage2[5] |= CAN_ignition_rly_BIT;
//	if (horn == GPIO_PIN_SET) 				VCU_StatusPackage2[4] |= CAN_horn_BIT;
//	if (longBeam == GPIO_PIN_SET) 			VCU_StatusPackage2[4] |= CAN_longBeam_BIT;
//	if (output_getState(&forceShutdown_rly) == GPIO_PIN_SET) 	VCU_StatusPackage2[4] |= CAN_forceShutdown_rly_BIT;
	if (output_getState(&speedMode_High) 	== GPIO_PIN_SET) 	VCU_StatusPackage2[5] |= CAN_speedMode_High_BIT;
	if (output_getState(&speedMode_Low) 	== GPIO_PIN_SET) 	VCU_StatusPackage2[5] |= CAN_speedMode_Low_BIT;
	if (output_getState(&speedMode_Reverse)== GPIO_PIN_SET) 	VCU_StatusPackage2[5] |= CAN_speedMode_Reverse_BIT;
//	if (speedMode_Cruise == GPIO_PIN_SET) 	VCU_StatusPackage2[4] |= CAN_speedMode_Cruise_BIT;

	// Byte 6 data : seatLock Status
	// Byte 7 data : steeringLock_Status
	VCU_StatusPackage2[7] = (uint8_t)steeringLock_status;

}

void CAN_Encode_VCU_Request1(void){
	// Byte 0 data : VCU Read BMS Serial Number
	// Byte 1 data : VCU Read BMS Firmware Serial Number
	// Byte 2 data : VCU Read MCU Serial Number
	// Byte 3 data : VCU Read MCU Firmware Serial Number
	// Byte 4 data : VCU Reserved 0
	// Byte 5 data : VCU Reserved 1
	// Byte 6 data : VCU Reserved 2
	// Byte 7 data : VCU Reserved 3
}

void CAN_Encode_VCU_Request2(void){
	// Byte 0 data : VCU Set Derating Discharge Voltage
	// Byte 1 data : VCU Set Derating Discharge Voltage
	// Byte 2 data : VCU Set Derating Discharge Current
	// Byte 3 data : VCU Set Derating Discharge Current
	// Byte 4 data : VCU Set Derating Discharge Temperature
	// Byte 5 data : VCU Set Derating Discharge Temperature
	// Byte 6 data : VCU Set Derating Discharge SOC
	// Byte 7 data : VCU Set Derating Discharge SOC
}

void CAN_Encode_VCU_Request3(void){
	// Byte 0 data : VCU Set Under-Voltage
	// Byte 1 data : VCU Set Under-Voltage
	// Byte 2 data : VCU Set Over-Voltage
	// Byte 3 data : VCU Set Over-Voltage
	// Byte 4 data : VCU Set Over-Current
	// Byte 5 data : VCU Set Over-Current
	// Byte 6 data : VCU Set Charge MOSFET Control
	// Byte 7 data : VCU Set Discharge MOSFET Control
}

void CAN_Encode_VCU_Request4(void){
	// Byte 0 data : VCU Set Under-Voltage Protection
	// Byte 1 data : VCU Set Under-Voltage Protection
	// Byte 2 data : VCU Set Over-Voltage Protection
	// Byte 3 data : VCU Set Over-Voltage Protection
	// Byte 4 data : VCU Set Under-Voltage Protection
	// Byte 5 data : VCU Set Under-Voltage Protection
	// Byte 6 data : VCU Set Regenerative Limit Current
	// Byte 7 data : VVCU Set Regenerative Limit Current
}

void CAN_Encode_VCU_displaySetting1(void){
	// Byte 0 data : VCU Set Date : Year
	// Byte 1 data : VCU Set Date : Year
	// Byte 2 data : VCU Set Date : Month
	// Byte 3 data : VCU Set Date : Day
	// Byte 4 data : VCU Set Time : Hour
	// Byte 5 data : VCU Set Time : Minute
	// Byte 6 data : VCU Set Time : Second
	// Byte 7 data : VCU Reserved 0
}
void CAN_Encode_VCU_displaySetting2(void){
	// Byte 0 data : VCU Set Display : Power
	// Byte 1 data : VCU Set Display : Sleep
	// Byte 2 data : VCU Set Display : Restart
	// Byte 3 data : VCU Set Display : Mode
	// Byte 4 data : VCU Set Display : Touchscreen Status
	// Byte 5 data : VCU Reserved 0
	// Byte 6 data : VCU Reserved 1
	// Byte 7 data : VCU Reserved 2
}

void CAN_Encode_VCU_vehiclePosition(void){
	// Byte 0 data : GNSS Latitude
	// Byte 1 data : GNSS Latitude
	// Byte 2 data : GNSS Latitude
	// Byte 3 data : GNSS Latitude
	// Byte 4 data : GNSS Longitude
	// Byte 5 data : GNSS Longitude
	// Byte 6 data : GNSS Longitude
	// Byte 7 data : GNSS Longitude
}

void CAN_Encode_VCU_toBMScmd(void){
	// Byte 0 data : VCU Data Request to BMS
	// Byte 1 data : VCU Data Request to BMS (Counter)
	// Byte 2 data : VCU Data Request to BMS (Counter)
	// Byte 3 data : VCU Reserved 0
	// Byte 4 data : VCU Reserved 1
	// Byte 5 data : VCU Reserved 2
	// Byte 6 data : VCU Reserved 3
	// Byte 7 data : VCU Reserved 4
}
