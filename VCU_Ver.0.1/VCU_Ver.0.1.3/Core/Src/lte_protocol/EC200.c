/*
 * EC200.c
 *
 *  Created on: Oct 12, 2026
 *      Author: fahmif_imoto
 */

#include "lte_protocol/EC200.h"
#include "IO_control/output_ctrl.h"

#include <stdio.h>
#include <string.h>

//debug variable
volatile uint8_t LTE_debug_power_step 		= 0U;
volatile uint8_t LTE_debug_at_attempt 		= 0U;
volatile uint8_t LTE_debug_at_result 		= EC200_ERROR;

volatile uint8_t LTE_debug_mqtt_step		= 0U;
volatile uint8_t LTE_debug_ip_check 		= 0U;
volatile uint8_t LTE_debug_ip_first_char	= 0U;
volatile uint8_t LTE_debug_ip_prefix_found	= 0U;

//private variables
static uint16_t mqtt_msg_id = 1U;

// Private function prototypes
static void LTE_EC200_ClearBuffer(EC200_Handler_t *hec200);
static uint8_t LTE_EC200_IsStringTerminated (const char *str, size_t size);
static uint8_t LTE_EC200_HasValidIPAddress (EC200_Handler_t *hec200);
static EC200_Status_t LTE_EC200_WaitForLineEnd(EC200_Handler_t *hec200,uint32_t timeout);

/* ========================================================= */
/* Initialization ========================================== */
/* ========================================================= */
EC200_Status_t LTE_EC200_Init(EC200_Handler_t *hec200, UART_HandleTypeDef *huart,
                         const GPIO_Config *pwrKey){
    if (hec200 == NULL || huart == NULL || pwrKey == NULL || pwrKey->port == NULL)	return EC200_ERROR;

    hec200->huart 			= huart;
    hec200->powerkey_port 	= pwrKey->port;
    hec200->powerkey_pin 	= pwrKey->pin;
    hec200->is_initialized 	= 0U;
    hec200->is_connected 	= 0U;
    hec200->timeout 		= 5000U; // default timeout 5 seconds

    // Initialize powerkey pin
    output_setState(pwrKey, GPIO_PIN_RESET);

    LTE_EC200_ClearBuffer(hec200);

    return EC200_OK;
}

/* ========================================================= */
/* Power Control =========================================== */
/* ========================================================= */
EC200_Status_t LTE_EC200_PowerOn(EC200_Handler_t *hec200){
	EC200_Status_t status;
	uint8_t attempt;

	if (hec200 == NULL || hec200->huart == NULL || hec200->powerkey_port == NULL){
		LTE_debug_power_step = 0U;
		return EC200_ERROR;
	}

    GPIO_Config pwrKey = {
    		.port	= hec200->powerkey_port,
			.pin	= hec200->powerkey_pin
    };

    hec200->is_initialized 	= 0U;
    hec200->is_connected 	= 0U;
    LTE_debug_power_step 	= 1U;

    output_setState(&pwrKey, GPIO_PIN_RESET);
    HAL_Delay(100);
    // Power on sequence for EC200
    LTE_debug_power_step 	= 2U;
    output_setState(&pwrKey, GPIO_PIN_SET);
    LTE_EC200_Delay(1000U);

    LTE_debug_power_step = 3U;
    output_setState(&pwrKey, GPIO_PIN_RESET);

    LTE_debug_power_step = 4U;
    LTE_EC200_Delay(5000U);

    LTE_debug_power_step = 5U;
    for (attempt = 0U; attempt < 10U; attempt++){
    	LTE_debug_at_attempt = attempt + 1U;

    	status = LTE_EC200_SendATCommand(hec200, "AT", 1000U);

    	if (status == EC200_OK){
    		status = LTE_EC200_WaitForResponse(hec200, "OK" , 1000U);
    	}

    	LTE_debug_at_result = status;

    	if (status == EC200_OK){
    		hec200->is_initialized = 1U;
    		LTE_debug_power_step = 6U;
    		return EC200_OK;
    	}
    	HAL_Delay(500U);
    }
    hec200->is_initialized 	= 0U;
    LTE_debug_power_step	= 7U;

    return EC200_TIMEOUT;
}

EC200_Status_t LTE_EC200_PowerOff(EC200_Handler_t *hec200){
	if (hec200 == NULL || hec200->huart == NULL || hec200->powerkey_port == NULL) return EC200_ERROR;

	GPIO_Config pwrKey = {
			.port	= hec200->powerkey_port,
			.pin 	= hec200->powerkey_pin
	};

    // Power off sequence
    output_setState(&pwrKey, GPIO_PIN_SET);
    LTE_EC200_Delay(3000);
    output_setState(&pwrKey, GPIO_PIN_RESET);
    LTE_EC200_Delay(5000);

    hec200->is_initialized = 0;
    hec200->is_connected = 0;

    LTE_EC200_ClearBuffer(hec200);
    return EC200_OK;
}

/* ========================================================= */
/* AT Command Transmission ================================= */
/* ========================================================= */
EC200_Status_t LTE_EC200_SendATCommand(EC200_Handler_t *hec200, const char *command,
                                  uint32_t timeout){
    if (hec200 == NULL || hec200->huart == NULL || command == NULL) return EC200_ERROR;

    LTE_EC200_ClearBuffer(hec200);
    hec200->timeout = timeout;

    // Send command
    if (HAL_UART_Transmit(hec200->huart, (uint8_t*)command, strlen(command), timeout) != HAL_OK)	return EC200_ERROR;

    // Send carriage return and new line
    if (HAL_UART_Transmit(hec200->huart, (uint8_t*)"\r\n", 2U, timeout) != HAL_OK)					return EC200_ERROR;

    return EC200_OK;
}

/* ========================================================= */
/* AT RESPONSE RECEPTIN ==================================== */
/* ========================================================= */
EC200_Status_t LTE_EC200_WaitForResponse(EC200_Handler_t *hec200, const char *expected_response,
                                    uint32_t timeout){
    uint32_t start_time;
    uint16_t index;
    HAL_StatusTypeDef uart_status;

    if (hec200 == NULL || hec200->huart == NULL || expected_response == NULL)		return EC200_ERROR;

    if (LTE_EC200_IsStringTerminated(
            hec200->response_buffer,
            sizeof(hec200->response_buffer)) == 0U)									return EC200_ERROR;

    index = (uint16_t)strlen(hec200->response_buffer);
    if (index >= sizeof(hec200->response_buffer)) 									return EC200_ERROR;

    start_time = HAL_GetTick();


    while ((HAL_GetTick() - start_time) < timeout)
    {
        // Receive data
        uint8_t byte = 0U;
        uart_status = HAL_UART_Receive(hec200->huart, &byte, 1U, 10U);

        if (uart_status == HAL_OK){
        	if (index >= sizeof(hec200->response_buffer) - 1U)		return EC200_ERROR;

            hec200->response_buffer[index++] = (char)byte;
            hec200->response_buffer[index] = '\0';

            if(strstr(hec200->response_buffer, expected_response)!= NULL)			return EC200_OK;

            if(byte == '\n' && strstr(hec200->response_buffer, "ERROR") != NULL) 	return EC200_ERROR;
        }
        else if(uart_status != HAL_TIMEOUT)		return EC200_ERROR;
    }
	if (strstr(hec200->response_buffer, "ERROR") != NULL) 							return EC200_ERROR;

    return EC200_TIMEOUT;
}

static EC200_Status_t LTE_EC200_WaitForLineEnd(EC200_Handler_t *hec200,uint32_t timeout){
    uint32_t start_time;
    size_t index;

    if (hec200 == NULL || hec200->huart == NULL)									return EC200_ERROR;

    index = strlen(hec200->response_buffer);
    if (index >= sizeof(hec200->response_buffer))									return EC200_ERROR;

    start_time = HAL_GetTick();

    while ((HAL_GetTick() - start_time) < timeout){
        uint8_t byte;
        HAL_StatusTypeDef uart_status;

        uart_status = HAL_UART_Receive(hec200->huart, &byte, 1U, 10U);
        if (uart_status == HAL_OK)
        {
            if (index >= sizeof(hec200->response_buffer) - 1U)						return EC200_ERROR;

            hec200->response_buffer[index++] = (char)byte;
            hec200->response_buffer[index] = '\0';

            if (byte == '\n')														return EC200_OK;
        }
        else if (uart_status != HAL_TIMEOUT)							            return EC200_ERROR;

    }

    return EC200_TIMEOUT;
}

/* ========================================================= */
/* Network Check =========================================== */
/* ========================================================= */
EC200_Status_t LTE_EC200_CheckNetwork(EC200_Handler_t *hec200)
{
    EC200_Status_t status;

    if(hec200 == NULL || hec200->huart == NULL || hec200->is_initialized == 0U)	return EC200_NOT_READY;


    // 1.  Check if module is responding
    status = LTE_EC200_SendATCommand(hec200, "AT", 1000);
    if (status != EC200_OK) return status;
    if (LTE_EC200_WaitForResponse(hec200, "OK", 2000) != EC200_OK) return  EC200_ERROR;

    // 2. Check SIM status
    status = LTE_EC200_SendATCommand(hec200, "AT+CPIN?", 2000);
    if (status != EC200_OK) return status;
    if (LTE_EC200_WaitForResponse(hec200, "READY", 3000) != EC200_OK) return EC200_NOT_READY;

    // 3. Check signal quality
    status = LTE_EC200_SendATCommand(hec200, "AT+CSQ", 2000);
    if (status != EC200_OK) return status;
    if (LTE_EC200_WaitForResponse(hec200, "OK", 3000) != EC200_OK) return EC200_ERROR;

    // 4. Check network registration
    status = LTE_EC200_SendATCommand(hec200, "AT+CREG?", 2000);
    if (status != EC200_OK) return status;
    if (LTE_EC200_WaitForResponse(hec200, "0,1", 3000) != EC200_OK) return EC200_NETWORK_ERROR;

    // 5. Check GPRS attachment
    status = LTE_EC200_SendATCommand(hec200, "AT+CGATT?", 2000);
    if (status != EC200_OK) return status;
    if (LTE_EC200_WaitForResponse(hec200, "+CGATT: 1", 3000) != EC200_OK) return EC200_NETWORK_ERROR;

    return EC200_OK;
}

/* ========================================================= */
/* MQTT Connection ========================================= */
/* ========================================================= */
EC200_Status_t LTE_EC200_ConnectMQTT(EC200_Handler_t *hec200, EC200_MQTT_Config_t *mqtt_config)
{
    EC200_Status_t status;
    char command[256];
    int len;

    //1. Parameter validation
    LTE_debug_mqtt_step = 1U;
    if (hec200 == NULL || mqtt_config == NULL)    					return EC200_ERROR;
    if (hec200->huart == NULL || hec200->is_initialized == 0U)		return EC200_NOT_READY;
    if (!LTE_EC200_IsStringTerminated(mqtt_config->server, sizeof(mqtt_config->server)) 		||
    	!LTE_EC200_IsStringTerminated(mqtt_config->client_id, sizeof(mqtt_config->client_id))	||
		!LTE_EC200_IsStringTerminated(mqtt_config->username, sizeof(mqtt_config->username))		||
		!LTE_EC200_IsStringTerminated(mqtt_config->password, sizeof(mqtt_config->password))
    ) return EC200_ERROR;
    if (mqtt_config->server[0] 		== '\0' ||
    	mqtt_config->client_id[0] 	== '\0' ||
		mqtt_config->port 			== 0U
	) return EC200_ERROR;

    hec200->is_connected = 0U;

    //2. Enable detailed modem errors
    LTE_debug_mqtt_step = 2U;
    status = LTE_EC200_SendATCommand(hec200, "AT+CMEE=2", 3000);

    if (status != EC200_OK)											return status;

    if (LTE_EC200_WaitForResponse(hec200, "OK", 3000) != EC200_OK)	return EC200_ERROR;

    //3. Configure PDP context
    LTE_debug_mqtt_step = 3U;
//    status = LTE_EC200_SendATCommand(hec200, "AT+CGDCONT=1,\"IP\",\"indosatiot\"", 5000);
//    status = LTE_EC200_SendATCommand(hec200, "AT+CGDCONT?", 5000U);
//    if (status != EC200_OK) 										return status;
//    if (LTE_EC200_WaitForResponse(hec200, "OK", 5000) != EC200_OK) 	return EC200_ERROR;

    //4. Read PDP configuration
    LTE_debug_mqtt_step = 4U;
    status = LTE_EC200_SendATCommand(hec200, "AT+CGDCONT?", 5000);
    if (status != EC200_OK)											return status;
    if (LTE_EC200_WaitForResponse(hec200, "OK", 5000) != EC200_OK)	return EC200_ERROR;

    //5. Activate PDP context
    LTE_debug_mqtt_step = 5U;
    status = LTE_EC200_SendATCommand(hec200, "AT+CGACT?", 10000);
    if (status != EC200_OK) 										return status;

    status = LTE_EC200_WaitForResponse(hec200, "OK", 5000U);
    if (status != EC200_OK) 										return status;

    if (strstr(hec200->response_buffer, "+CGACT: 1,1") == NULL){
    	status = LTE_EC200_SendATCommand(hec200, "AT+CGACT=1,1", 10000U);
    	if (status != EC200_OK) 									return status;

    	status = LTE_EC200_WaitForResponse(hec200, "OK", 10000U);
    	if (status != EC200_OK) 									return status;
    }

    // 6. Get IP address
    LTE_debug_mqtt_step = 6U;
    status = LTE_EC200_SendATCommand(hec200, "AT+CGPADDR=1", 5000U);
    if (status != EC200_OK) 										return status;
    status = LTE_EC200_WaitForResponse(hec200, "OK", 5000U);
    if (status != EC200_OK){
    	LTE_debug_ip_check = 2U;
    	return status;
    }

    LTE_debug_ip_check =  LTE_EC200_HasValidIPAddress(hec200) ? 4U:3U;
    if (LTE_debug_ip_check == 3U)									return EC200_NETWORK_ERROR;

    // 7. Configure MQTT version
    LTE_debug_mqtt_step = 7U;
    status = LTE_EC200_SendATCommand(hec200, "AT+QMTCFG=\"version\",0,4", 3000);
    if (status != EC200_OK) 										return status;
    if (LTE_EC200_WaitForResponse(hec200, "OK", 3000) != EC200_OK) 	return EC200_ERROR;

    // 8. Open MQTT connection
    LTE_debug_mqtt_step = 8U;

    len = snprintf(
        command,
        sizeof(command),
        "AT+QMTOPEN=0,\"%s\",%u",
        mqtt_config->server,
        (unsigned int)mqtt_config->port
    );

    if (len < 0 || (size_t)len >= sizeof(command))
        return EC200_ERROR;

    status = LTE_EC200_SendATCommand(
        hec200, command, 30000U);

    if (status != EC200_OK)
        return status;

    // Wait until the QMTOPEN URC prefix appears
    status = LTE_EC200_WaitForResponse(
        hec200, "+QMTOPEN: 0,", 30000U);

    if (status != EC200_OK)
        return status;

    // Complete the URC line before examining its result
    status = LTE_EC200_WaitForLineEnd(
        hec200, 5000U);

    if (status != EC200_OK)
        return status;

    // 8.1. Handle occupied MQTT client
    if (strstr(
            hec200->response_buffer,
            "+QMTOPEN: 0,2") != NULL)
    {
        status = LTE_EC200_SendATCommand(
            hec200, "AT+QMTCLOSE=0", 5000U);

        if (status != EC200_OK)
            return status;

        status = LTE_EC200_WaitForResponse(
            hec200, "+QMTCLOSE: 0,0", 10000U);

        if (status != EC200_OK)
            return status;

        // Retry QMTOPEN once
        status = LTE_EC200_SendATCommand(
            hec200, command, 30000U);

        if (status != EC200_OK)
            return status;

        status = LTE_EC200_WaitForResponse(
            hec200, "+QMTOPEN: 0,", 30000U);

        if (status != EC200_OK)
            return status;

        status = LTE_EC200_WaitForLineEnd(
            hec200, 5000U);

        if (status != EC200_OK)
            return status;
    }

    // Accept only an explicit successful result
    if (strstr(
            hec200->response_buffer,
            "+QMTOPEN: 0,0") == NULL)
    {
        return EC200_NETWORK_ERROR;
    }
//    LTE_debug_mqtt_step = 8U;
//    len = snprintf(command, sizeof(command), "AT+QMTOPEN=0,\"%s\",%u",
//             mqtt_config->server, mqtt_config->port);
//
//    if (len < 0 || (size_t)len >= sizeof(command))					return status;
//
//    status = LTE_EC200_SendATCommand(hec200, command, 30000U);
//
//    if (status != EC200_OK) return status;
//
//    status = LTE_EC200_WaitForResponse(hec200, "+QMTOPEN: 0,", 30000U);
//    if (status != EC200_OK) return status;
//
//    // 8.1. Check whether client ID 0 is occupied
//    if (strstr(hec200->response_buffer, "+QMTOPEN: 0,2") != NULL){
//    	// 8.2. Close the occupied MQTT client
//    	status = LTE_EC200_SendATCommand(hec200, "AT+QMTCLOSE=0", 5000U);
//    	if (status != EC200_OK)										return status;
//
//    	status = LTE_EC200_WaitForResponse(hec200, "+QMTCLOSE: 0,0", 10000U);
//    	if (status != EC200_OK)										return status;
//
//    	// 8.3. Retry QMTOPEN once
//    	status = LTE_EC200_SendATCommand(hec200, command, 30000U);
//    	if (status != EC200_OK)										return status;
//
//    	status = LTE_EC200_WaitForResponse(hec200, "+QMTOPEN: 0,", 30000U);
//    	if (status != EC200_OK)										return status;
//    }



    // Accept only a successful QMTOPEN result
    if (strstr(hec200->response_buffer, "+QMTOPEN: 0,0") == NULL)
    return EC200_NETWORK_ERROR;

    // 9. Connect to MQTT broker
    LTE_debug_mqtt_step = 9U;
    if(mqtt_config->username[0] != '\0'){
    	len = snprintf(
    			command,
				sizeof(command),
				"AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"",
	            mqtt_config->client_id,
	            mqtt_config->username,
	            mqtt_config->password
    	);
    }

    else{
    	len = snprintf(
    			command,
				sizeof(command),
				"AT+QMTCONN=0,\"%s\"",
	            mqtt_config->client_id);
    }

    if (len < 0 || (size_t)len >= sizeof(command))				return EC200_ERROR;

    status = LTE_EC200_SendATCommand(hec200, command, 30000U);

    if (status != EC200_OK)										return status;

    if (LTE_EC200_WaitForResponse(hec200, "+QMTCONN: 0,0,0", 30000U) != EC200_OK) return EC200_ERROR;

    //9. MQTT connected
    hec200->is_connected = 1U;
    return EC200_OK;
}

/* ========================================================= */
/* MQTT Publish ============================================ */
/* ========================================================= */
EC200_Status_t LTE_EC200_PublishMQTT(EC200_Handler_t *hec200, const char *topic,
                                const char *message, uint8_t qos){
    char command[128];
    char expected_response[32];
    EC200_Status_t status;
    uint16_t msg_id;
    int len;

    //1. parameter validation
	if (hec200 == NULL || topic == NULL || message == NULL)		return EC200_ERROR;
	if (hec200->huart ==NULL || hec200->is_connected == 0U)		return EC200_ERROR;
	if (topic[0] == '\0' || qos > 2)							return EC200_ERROR;

	//2. Allocate MQTT message ID
	msg_id = mqtt_msg_id++;

    if(mqtt_msg_id == 0){
    	mqtt_msg_id = 1;
    }

    //3. make an MQTT publish command
    //message format: AT+QMTPUB=<client_idx>,<msgID>,<qos>,<retain>,<topic>
    len = snprintf(
    		command,
			sizeof(command),
			"AT+QMTPUB=0,%u,%u,0,\"%s\"",
			(unsigned int)msg_id,
			(unsigned int)qos,
			topic);

    if (len < 0 || (size_t)len >= sizeof(command))				return EC200_ERROR;

    status = LTE_EC200_SendATCommand(hec200, command, 3000);
    if (status != EC200_OK) 									return status;

    //4. Wait for payload prompt
    status = LTE_EC200_WaitForResponse(hec200, ">", 5000);

    if (status != EC200_OK)										return status;

    //5. send payload
    status = (HAL_UART_Transmit(hec200->huart, (uint8_t *)message, strlen(message), 3000) == HAL_OK) ? EC200_OK : EC200_ERROR;
    if (status != EC200_OK)										return status;

    //6. Send Ctrl+Z to finish payload
    uint8_t ctrl_z = 0x1A;

    if(HAL_UART_Transmit(hec200->huart, &ctrl_z, 1, 3000) != HAL_OK){
    	return EC200_ERROR;
    }

    //7. Wait for publish confirmation
    //URC format: +QMTPUB: <client_idx>,<msgID>,<result>
    len = snprintf(
        expected_response,
        sizeof(expected_response),
        "+QMTPUB: 0,%u,0",
        (unsigned int)msg_id);

    if (len < 0 || (size_t)len >= sizeof(expected_response))	return EC200_ERROR;

    status = LTE_EC200_WaitForResponse(hec200, expected_response, 10000);

    if (status != EC200_OK)										return status;

    //8. Publish success
    return EC200_OK;

}

/* ========================================================= */
/* MQTT Disconnected ======================================= */
/* ========================================================= */
EC200_Status_t LTE_EC200_DisconnectMQTT(EC200_Handler_t *hec200)
{
    EC200_Status_t status;

    if (hec200 == 0U || hec200->huart == NULL)			return EC200_ERROR;
    if (hec200->is_connected == 0U)						return EC200_OK;

    // Close MQTT connection
    status = LTE_EC200_SendATCommand(hec200, "AT+QMTDISC=0", 10000U);
    if (status != EC200_OK) 								return status;

    if (LTE_EC200_WaitForResponse(hec200, "OK", 10000) == EC200_OK)
    {
        hec200->is_connected = 0;
        return EC200_OK;
    }

    return EC200_ERROR;
}


/* ========================================================= */
/* Private Helpers ========================================= */
/* ========================================================= */
static void LTE_EC200_ClearBuffer(EC200_Handler_t *hec200)
{
    if (hec200 != NULL)
    {
        memset(hec200->response_buffer, 0, sizeof(hec200->response_buffer));
    }
}

static uint8_t LTE_EC200_IsStringTerminated (const char *str, size_t size){
	if (str == NULL || size == 0U)						return 0U;
	return (memchr(str, '\0', size) != NULL) ? 1U:0U;
}

static uint8_t LTE_EC200_HasValidIPAddress (EC200_Handler_t *hec200){
	const char *ip;
	if (hec200 ==NULL)									return 0U;
	ip = strstr(hec200->response_buffer, "+CGPADDR: 1,");
	LTE_debug_ip_prefix_found = (ip != NULL) ? 1U : 0U;

	if (ip == NULL)										return 0U;

	ip += strlen("+CGPADDR: 1,");

	while (*ip == ' ' || *ip == '\t')
		ip++;

	LTE_debug_ip_first_char = (uint8_t)*ip;

	if (*ip == '"')
	ip++;

	//Reject missing address or an address of 0.0.0.0.
	if (*ip < '0' || *ip > '9')							return 0U;
	if (strncmp(ip, "0.0.0.0", 7U) == 0 && (ip[7] == '"' || ip[7] == '\r' || ip[7] == '\n' || ip[7] == '\0'))
		return 0U;

	return 1U;
}

/* ========================================================= */
/* Delay =================================================== */
/* ========================================================= */

void LTE_EC200_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}
