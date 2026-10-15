/*
 * lte_handler.c
 *
 *  Created on: Oct 12, 2026
 *      Author: fahmif_imoto
 */


#include "lte_protocol/lte_handler.h"
#include "LTE_config.h"
#include "IO_control/output_ctrl.h"
#include "can_protocol/CAN_LTE_dataForward.h"
#include <string.h>

//external UART
extern UART_HandleTypeDef huart1;

//EC200 object
EC200_Handler_t ec200;
EC200_MQTT_Config_t mqtt_config;

//status init
EC200_Status_t LTE_handler_init 	= EC200_ERROR;
EC200_Status_t LTE_handler_power	= EC200_ERROR;
EC200_Status_t LTE_handler_network	= EC200_ERROR;
EC200_Status_t LTE_handler_mqtt		= EC200_ERROR;

volatile EC200_Status_t LTE_debug_poweron_wait = EC200_ERROR;

//debug
uint8_t LTE_handler_powerkey_hi 	= 0;
uint8_t LTE_handler_powerkey_lo 	= 0;
uint8_t LTE_handler_ready_received 	= 0;

//debug AT command
volatile uint8_t LTE_debug_AT		= 0;
volatile uint8_t LTE_debug_CPIN		= 0;
volatile uint8_t LTE_debug_CSQ		= 0;
volatile uint8_t LTE_debug_CREG		= 0;
volatile uint8_t LTE_debug_CGATT	= 0;

//debug PDP
volatile uint8_t LTE_debug_CGDCONT = 0U;
volatile uint8_t LTE_debug_CGACT   = 0U;
volatile uint8_t LTE_debug_CGPADDR = 0U;

volatile EC200_Status_t LTE_result_CGDCONT = EC200_ERROR;
volatile EC200_Status_t LTE_result_CGACT   = EC200_ERROR;
volatile EC200_Status_t LTE_result_CGPADDR = EC200_ERROR;

//debug TCP
volatile uint8_t LTE_debug_TCP 						= 0U;
volatile uint8_t LTE_tcp_result 					= 0U;

volatile uint8_t LTE_debug_QISTATE 					= 0U;
volatile uint8_t LTE_debug_QICLOSE 					= 0U;
volatile uint8_t LTE_debug_QMTOPEN_TEST 			= 0U;

//debug MQTT
volatile uint8_t 	LTE_debug_MQTT					= 0U;
volatile uint8_t 	LTE_debug_MQTT_step				= 0U;
volatile uint32_t 	LTE_debug_mqtt_trigger_count 	= 0U;

//internal variable
static uint8_t LTEhandler_ready		= 0U;
static uint32_t reconnect_tick 		= 0U;

#define LTE_RECONNECT_INTERVAL_MS	10000U

//CAN data forward configuration
#define LTE_CAN_PUBLISH_INTERVAL_MS    1000U
#define LTE_CAN_PAYLOAD_SIZE           4096U

static uint32_t canPublishTick = 0U;
static uint32_t canBatchID = 0U;

static char canMqttPayload[LTE_CAN_PAYLOAD_SIZE];

//internal function
static EC200_Status_t LTEhandler_checkNetwork(void);
static void LTEhandler_processDebug(void);
static void LTEhandler_tryReconnect(void);
static void LTEhandler_publishCANQueue(void);



/* ========================================================= */
/* Initialization ========================================== */
/* ========================================================= */
void LTEhandler_init(void){
	ec200.is_initialized 		= 0U;
	ec200.is_connected 			= 0U;

	LTEhandler_ready 			= 0U;

	canPublishTick 				= 0U;
	canBatchID 					= 0U;

    LTE_handler_powerkey_hi 	= 0U;
    LTE_handler_powerkey_lo 	= 0U;
    LTE_handler_ready_received 	= 0U;

    LTE_handler_init 			= EC200_ERROR;
    LTE_handler_power 			= EC200_ERROR;
    LTE_handler_network 		= EC200_ERROR;
    LTE_handler_mqtt 			= EC200_ERROR;


	//1. initialize EC200 driver
//	LTE_handler_init 		= LTE_EC200_Init(&ec200, &huart1, LTE_ON_GPIO_Port, LTE_ON_Pin);
	LTE_handler_init 		= LTE_EC200_Init(&ec200, &huart1, &LTE_pwrKey);

	if (LTE_handler_init != EC200_OK){
		return;
	}

	//2. PWRKEY turn on and AT response verification
	LTE_handler_power			= LTE_EC200_PowerOn(&ec200);

	if (LTE_handler_power != EC200_OK){
		ec200.is_initialized 	= 0U;
		LTEhandler_ready		= 0U;
		return;
	}
	LTE_handler_ready_received	= 1U;

	//3. network check
	LTE_handler_network 		= LTEhandler_checkNetwork();

	if(LTE_handler_network != EC200_OK){
		LTEhandler_ready 		= 0U;
		return;
	}

	//4. MQTT configuration
	memset(&mqtt_config, 0, sizeof(mqtt_config));

	strcpy(mqtt_config.server, "prodbroker.imoto.tech");
	mqtt_config.port			= 1883U;
	strcpy(mqtt_config.client_id, "EC200U_VCU_866082073348494");
	strcpy(mqtt_config.topic, "imoto_vcu/v1/866082073348494");

	strcpy(mqtt_config.username, "imoto_fahmi");
	strcpy(mqtt_config.password, "imoto2026");

	//5. connect MQTT
	LTE_handler_mqtt = LTE_EC200_ConnectMQTT(&ec200, &mqtt_config);
	if(LTE_handler_mqtt != EC200_OK){
		ec200.is_connected = 0U;
		LTEhandler_ready = 0U;
		return;
	}
	ec200.is_connected 			= 1U;
	LTEhandler_ready			= 1U;

	reconnect_tick				= HAL_GetTick();
	canPublishTick				= HAL_GetTick() - LTE_CAN_PUBLISH_INTERVAL_MS;
}

/* ========================================================= */
/* Check Network =========================================== */
/* ========================================================= */
static EC200_Status_t LTEhandler_checkNetwork(void){
	EC200_Status_t status;

	//modem response
	status 		 = LTE_EC200_SendATCommand(&ec200, "AT", 3000U);
	if(status 	!= EC200_OK) 	return status;
	status		 = LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	if(status 	!= EC200_OK) 	return status;

	//SIM status
	status 		 = LTE_EC200_SendATCommand(&ec200, "AT+CPIN?", 3000U);
	if(status 	!= EC200_OK) 	return status;
	status		 = LTE_EC200_WaitForResponse(&ec200, "READY", 3000U);
	if(status 	!= EC200_OK) 	return EC200_NOT_READY;

	//signal quality
	status 		 = LTE_EC200_SendATCommand(&ec200, "AT+CSQ", 3000U);
	if(status 	!= EC200_OK) 	return status;
	status		 = LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	if(status 	!= EC200_OK) 	return status;

	//network registration
	status 		 = LTE_EC200_SendATCommand(&ec200, "AT+CREG?", 3000U);
	if(status 	!= EC200_OK) 	return status;
	status		 = LTE_EC200_WaitForResponse(&ec200, "0,1", 3000U);
	if(status 	!= EC200_OK) 	return EC200_NETWORK_ERROR;

	//package service attachment
	status 		 = LTE_EC200_SendATCommand(&ec200, "AT+CGATT?", 3000U);
	if(status 	!= EC200_OK) 	return status;
	status		 = LTE_EC200_WaitForResponse(&ec200, "+CGATT: 1", 3000U);
	if(status 	!= EC200_OK) 	return EC200_NETWORK_ERROR;

	return EC200_OK;
}

/* ========================================================= */
/* Main Task =============================================== */
/* ========================================================= */
void LTEhandler_task(void){
	if(!LTEhandler_ready)		return;
	LTEhandler_processDebug();

	if(!ec200.is_connected){
		LTEhandler_tryReconnect();
		return;
	}

	LTEhandler_publishCANQueue();
}

static void LTEhandler_processDebug(void){
	//check modem
	if (LTE_debug_AT){
		LTE_debug_AT = 0U;
		LTE_EC200_SendATCommand(&ec200, "AT", 3000U);
		LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	}

	//CPIN
	if (LTE_debug_CPIN){
		LTE_debug_CPIN = 0U;
		LTE_EC200_SendATCommand(&ec200, "AT+CPIN?", 3000U);
		LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	}

	//CSQ
	if (LTE_debug_CSQ){
		LTE_debug_CSQ = 0U;
		LTE_EC200_SendATCommand(&ec200, "AT+CSQ", 3000U);
		LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	}

	//CREG
	if (LTE_debug_CREG){
		LTE_debug_CREG = 0U;
		LTE_EC200_SendATCommand(&ec200, "AT+CREG?", 3000U);
		LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	}

	//CGATT
	if (LTE_debug_CGATT){
		LTE_debug_CGATT = 0U;
		LTE_EC200_SendATCommand(&ec200, "AT+CGATT?", 3000U);
		LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	}

	//CGDCONT
	if (LTE_debug_CGDCONT != 0U){
	    LTE_debug_CGDCONT 	= 0U;
	    LTE_result_CGDCONT 	= LTE_EC200_SendATCommand(&ec200, "AT+CGDCONT?", 3000U);
	    if (LTE_result_CGDCONT == EC200_OK){
	    	LTE_result_CGDCONT = LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	    }
	}

	//CGACT
	if (LTE_debug_CGACT){
	    LTE_debug_CGACT 	= 0U;
	    LTE_result_CGACT 	= LTE_EC200_SendATCommand(&ec200, "AT+CGACT?", 3000U);
	    if (LTE_result_CGACT == EC200_OK){
	    	LTE_result_CGACT = LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	    }
	}

	//CGPADDR
	if (LTE_debug_CGPADDR){
	    LTE_debug_CGPADDR 	= 0U;
	    LTE_result_CGPADDR 	=LTE_EC200_SendATCommand(&ec200, "AT+CGPADDR=1", 3000U);
	    if (LTE_result_CGPADDR == EC200_OK){
	    	LTE_result_CGPADDR  = LTE_EC200_WaitForResponse(&ec200, "OK", 3000U);
	    }
	}

	//MQTT connection test
	if (LTE_debug_MQTT){
		LTE_debug_MQTT = 0U;
		LTE_debug_mqtt_trigger_count++;
		LTE_handler_mqtt = LTE_EC200_ConnectMQTT(&ec200, &mqtt_config);

		if (LTE_handler_mqtt == EC200_OK)	ec200.is_connected = 1U;
		else								ec200.is_connected = 0U;
	}

	//MQTT publish test

}


static void LTEhandler_publishCANQueue(void)
{
    uint32_t now = HAL_GetTick();
    uint8_t frameCount = 0U;

    //Nothing to publish
    if (CAN_forward_getQueueCount() == 0U)        						return;

    //Limit publishing frequency
    if ((uint32_t)(now - canPublishTick) < LTE_CAN_PUBLISH_INTERVAL_MS)	return;

    if (!ec200.is_connected)											return;

    /*
     * Build JSON from queued CAN frames.
     * The builder must limit the batch to
     * CAN_FORWARD_BATCH_SIZE.
     */
    if (CAN_forwardBuildPayload(
            canMqttPayload,
            sizeof(canMqttPayload),
            now,
            canBatchID,
            &frameCount) == 0U)
    {
        return;
    }

    if (frameCount == 0U)												return;

    canPublishTick = now;
//    LTE_publish_result = 0U;

    /* Publish CAN data to MQTT */
    if (LTE_EC200_PublishMQTT(
            &ec200,
            mqtt_config.topic,
            canMqttPayload,
            1U) == EC200_OK)
    {
        /*
         * Remove only the frames included in
         * the successfully published batch.
         */
        CAN_forwardCommit(frameCount);

        canBatchID++;
        LTE_publish_result = 1U;
    }
    else
    {
//        LTE_publish_result = 2U;
        ec200.is_connected = 0U;
    }
}


static void LTEhandler_tryReconnect(void){
    uint32_t now = HAL_GetTick();

    if ((uint32_t)(now - reconnect_tick) < LTE_RECONNECT_INTERVAL_MS)	return;
    reconnect_tick = now;

    LTE_handler_mqtt = LTE_EC200_ConnectMQTT(&ec200,&mqtt_config);
    if (LTE_handler_mqtt == EC200_OK){
        ec200.is_connected = 1U;
        canPublishTick = HAL_GetTick() - LTE_CAN_PUBLISH_INTERVAL_MS;
    }
    else{
        ec200.is_connected = 0U;
    }
}
