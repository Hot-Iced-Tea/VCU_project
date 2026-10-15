/*
 * ride_mode.h
 *
 *  Created on: Sep 18, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_SYSTEM_MODES_STATE_RIDE_MODE_H_
#define INC_SYSTEM_MODES_STATE_RIDE_MODE_H_

#include <stdint.h>

/* ========================================================= */
/* RIDE MODE =============================================== */
/* ========================================================= */

typedef enum{
	RideMode_ECO = 0U,
	RideMode_DRIVE,
	RideMode_SPORT,
	RideMode_REVERSE,
	RideMode_CRUISE,
}rideMode_t;

/* ========================================================= */
/* FUNCTION= =============================================== */
/* ========================================================= */

void rideMode_Init(void);
void rideMode_Task(void);
rideMode_t rideMode_Get(void);
rideMode_t rideMode_GetLastForwardMode(void);
extern volatile uint16_t CAN_vehicleSpeed_kmh;

#endif /* INC_SYSTEM_MODES_STATE_RIDE_MODE_H_ */
