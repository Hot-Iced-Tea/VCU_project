/*
 * EC200.h
 *
 *  Created on: Oct 12, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_LTE_PROTOCOL_EC200_H_
#define INC_LTE_PROTOCOL_EC200_H_

#ifndef INC_EC200_H_
#define INC_EC200_H_

#include "stm32h5xx_hal.h"
#include "LTE_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// debug variable
extern volatile uint8_t LTE_debug_power_step;
extern volatile uint8_t LTE_debug_at_attempt;
extern volatile uint8_t LTE_debug_at_result;

extern volatile uint8_t LTE_debug_mqtt_step;

// Status definitions
typedef enum {
    EC200_OK = 0,
    EC200_ERROR,
    EC200_TIMEOUT,
    EC200_NOT_READY,
    EC200_NETWORK_ERROR
} EC200_Status_t;

// MQTT Configuration
typedef struct {
    char server[128];
    uint16_t port;
    char client_id[64];
    char topic[128];
    char username[64];
    char password[64];
} EC200_MQTT_Config_t;

// EC200 Handler
typedef struct {
    UART_HandleTypeDef 	*huart;
    GPIO_TypeDef 		*powerkey_port;
    uint16_t 			powerkey_pin;
    uint8_t 			is_initialized;
    uint8_t 			is_connected;
    char 				response_buffer[512];
    uint32_t timeout;
} EC200_Handler_t;

// Function prototypes
EC200_Status_t LTE_EC200_Init(
		EC200_Handler_t *hec200,
		UART_HandleTypeDef *huart,
        const GPIO_Config *pwrKey);

//EC200_Status_t LTE_EC200_Init(
//		EC200_Handler_t *hec200,
//		UART_HandleTypeDef *huart,
//        GPIO_TypeDef *powerkey_port,
//		uint16_t powerkey_pin);

EC200_Status_t LTE_EC200_PowerOn(EC200_Handler_t *hec200);

EC200_Status_t LTE_EC200_PowerOff(EC200_Handler_t *hec200);

EC200_Status_t LTE_EC200_SendATCommand(
		EC200_Handler_t
		*hec200,
		const char *command,
		uint32_t timeout);

EC200_Status_t LTE_EC200_WaitForResponse(
		EC200_Handler_t *hec200,
		const char *expected_response,
		uint32_t timeout);

EC200_Status_t LTE_EC200_CheckNetwork(EC200_Handler_t *hec200);

EC200_Status_t LTE_EC200_ConnectMQTT(
		EC200_Handler_t *hec200,
		EC200_MQTT_Config_t *mqtt_config);

EC200_Status_t LTE_EC200_PublishMQTT(
		EC200_Handler_t *hec200,
		const char *topic,
        const char *message,
		uint8_t qos);

EC200_Status_t LTE_EC200_DisconnectMQTT(EC200_Handler_t *hec200);

// Helper functions
void LTE_EC200_Delay(uint32_t ms);

#endif /* INC_EC200_H_ */


#endif /* INC_LTE_PROTOCOL_EC200_H_ */
