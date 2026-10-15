/*
 * GPIO_config.h
 *
 *  Created on: Aug 29, 2026
 *      Author: Think
 */

#ifndef INC_CFG_GPIO_CONFIG_H_
#define INC_CFG_GPIO_CONFIG_H_

#include "main.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} GPIO_Config;

//input list
extern const GPIO_Config steeringLock_sw;
extern const GPIO_Config sideStand_sw;
extern const GPIO_Config speedMode_sw;
extern const GPIO_Config seatLock_sw;
extern const GPIO_Config forceShutdown_state;
extern const GPIO_Config ignition_state;

//output list
extern const GPIO_Config ignition_rly;
extern const GPIO_Config horn;
extern const GPIO_Config longBeam;
extern const GPIO_Config forceShutdown_rly;
extern const GPIO_Config speedMode_High;
extern const GPIO_Config speedMode_Low;
extern const GPIO_Config speedMode_Reverse;
extern const GPIO_Config speedMode_Cruise;
extern const GPIO_Config antiTheft;

extern const GPIO_Config steeringLock;
extern const GPIO_Config seatLock;

#endif /* INC_CFG_GPIO_CONFIG_H_ */
