/*
 * GPIO_config.c
 *
 *  Created on: Aug 29, 2026
 *      Author: Think
 */

#include "cfg/GPIO_config.h"

/* ==== INPUT CONFIGURATION ====*/
const GPIO_Config steeringLock_sw	 	= {IN1_GPIO_Port, IN1_Pin};
const GPIO_Config sideStand_sw			= {IN2_GPIO_Port, IN2_Pin};
const GPIO_Config speedMode_sw 			= {IN3_GPIO_Port, IN3_Pin};
const GPIO_Config seatLock_sw 			= {IN4_GPIO_Port, IN4_Pin};
const GPIO_Config forceShutdown_state	= {};
const GPIO_Config ignition_state		= {};

/* ==== OUTPUT CONFIGURATION ====*/
const GPIO_Config ignition_rly 		= {OUT1_GPIO_Port, OUT1_Pin};
const GPIO_Config horn 				= {OUT2_GPIO_Port, OUT2_Pin};
//const GPIO_Config longBeam 			= {OUT3_GPIO_Port, OUT3_Pin};
//const GPIO_Config forceShutdown_rly = {OUT4_GPIO_Port, OUT4_Pin};
const GPIO_Config speedMode_High	= {PWM1_GPIO_Port, PWM1_Pin};
const GPIO_Config speedMode_Low		= {PWM2_GPIO_Port, PWM2_Pin};
const GPIO_Config speedMode_Reverse	= {OUT4_GPIO_Port, OUT4_Pin};
//const GPIO_Config speedMode_Cruise	= {};
//const GPIO_Config antiTheft			= {};

// THIS IS USING 1-WIRE PROTOCOL
const GPIO_Config steeringLock		= {USART2_OUT_GPIO_Port, USART2_OUT_Pin};
const GPIO_Config seatLock			= {OUT3_GPIO_Port, OUT3_Pin};
//const GPIO_Config seatLock			= {USART2_OUT_GPIO_Port, USART2_OUT_Pin};
