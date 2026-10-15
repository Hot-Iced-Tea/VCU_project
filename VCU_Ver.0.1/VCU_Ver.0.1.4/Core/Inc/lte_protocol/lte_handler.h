/*
 * lte_handler.h
 *
 *  Created on: Oct 12, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_LTE_PROTOCOL_LTE_HANDLER_H_
#define INC_LTE_PROTOCOL_LTE_HANDLER_H_

#include "main.h"
#include "lte_protocol/EC200.h"

// EC200 object
extern EC200_Handler_t ec200;

//MQTT configuration
extern EC200_MQTT_Config_t mqtt_config;

//for live expression
extern EC200_Status_t LTE_handler_init;
extern EC200_Status_t LTE_handler_power;
extern EC200_Status_t LTE_handler_network;
extern EC200_Status_t LTE_handler_mqtt;

extern volatile EC200_Status_t LTE_debug_poweron_wait;

//debug status
extern uint8_t LTE_handler_powerkey_hi;
extern uint8_t LTE_handler_powerkey_lo;
extern uint8_t LTE_handler_ready_received;

//debug AT command
extern volatile uint8_t LTE_debug_AT;
extern volatile uint8_t LTE_debug_CPIN;
extern volatile uint8_t LTE_debug_CSQ;
extern volatile uint8_t LTE_debug_CREG;
extern volatile uint8_t LTE_debug_CGATT;

//debug PDP
extern volatile uint8_t LTE_debug_CGDCONT;
extern volatile uint8_t LTE_debug_CGACT;
extern volatile uint8_t LTE_debug_CGPADDR;

//debug TCP
extern volatile uint8_t LTE_debug_TCP;
extern volatile uint8_t LTE_tcp_result;

extern volatile uint8_t LTE_debug_QISTATE;
extern volatile uint8_t LTE_debug_QICLOSE;
extern volatile uint8_t LTE_debug_QMTOPEN_TEST;

//debug MQTT
extern volatile uint8_t LTE_debug_MQTT;
extern volatile uint8_t LTE_debug_MQTT_step;
extern volatile uint8_t LTE_debug_publish;
extern volatile uint8_t LTE_publish_result; //MQTT result

//Public Function
void LTEhandler_init(void);
void LTEhandler_task(void);

#endif /* INC_LTE_PROTOCOL_LTE_HANDLER_H_ */
