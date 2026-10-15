/*
 * CAN_lteForward.c
 *
 *  Created on: Oct 9, 2026
 *      Author: fahmif_imoto
 */

#include "main.h"
#include "can_protocol/CAN_LTE_dataForward.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#ifndef CAN_FORWARD_DEVICE_ID
#define CAN_FORWARD_DEVICE_ID "866082073348494"
#endif

static CAN_forwardFrame_t forwardQueue[CAN_FORWARD_QUEUE_SIZE];

static volatile uint16_t queueHead			= 0U;
static volatile uint16_t queueTail 			= 0U;
static volatile uint16_t queueCount 		= 0U;
static volatile uint32_t droppedFrameCount 	= 0U;

//CAN LTE debug variable
volatile uint32_t CAN_ForwardCaptureCallCount 	= 0U;
volatile uint32_t CAN_ForwardCaptureOK       	= 0U;
volatile uint32_t CAN_ForwardCaptureReject    	= 0U;
volatile uint32_t CAN_ForwardQueueFullCount   	= 0U;

volatile uint16_t CAN_ForwardQueueCount       	= 0U;
volatile uint16_t CAN_ForwardQueueHead        	= 0U;
volatile uint16_t CAN_ForwardQueueTail        	= 0U;
volatile uint32_t CAN_ForwardDroppedCount     	= 0U;

volatile uint32_t CAN_ForwardPayloadBuildOK   	= 0U;
volatile uint32_t CAN_ForwardPayloadBuildFail	= 0U;
volatile uint8_t  CAN_ForwardPayloadFrameCount	= 0U;
volatile uint16_t CAN_ForwardPayloadLength      = 0U;

static void CAN_forwardUpdateDebugQueue(void);


static uint32_t CAN_forwardLock(void){
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	__DMB();

	return primask;
}

static void CAN_forwardUnlock(uint32_t primask){
	__DMB();
	__set_PRIMASK(primask);
}

static uint8_t CAN_forwardIsAllowedID(uint32_t id, uint32_t idType){
	if (idType != FDCAN_EXTENDED_ID)	return 0U;

	switch (id){
		case 0x1800FCA1U:
		case 0x1801FCA1U:
		case 0x1808FCA1U:
			return 1U;

		default:
			return 0U;
	}
}

/* ========================================================= */
/* Initialization ========================================== */
/* ========================================================= */
void CAN_forward_Init(void){
	uint32_t primask = CAN_forwardLock();

	queueHead 			= 0U;
	queueTail 			= 0U;
	queueCount			= 0U;
	droppedFrameCount	= 0U;

	//Reset capture debug counters
    CAN_ForwardCaptureCallCount = 0U;
    CAN_ForwardCaptureOK        = 0U;
    CAN_ForwardCaptureReject    = 0U;
    CAN_ForwardQueueFullCount   = 0U;

    //Reset payload debug counters
    CAN_ForwardPayloadBuildOK     = 0U;
    CAN_ForwardPayloadBuildFail   = 0U;
    CAN_ForwardPayloadFrameCount  = 0U;
    CAN_ForwardPayloadLength      = 0U;

    CAN_forwardUpdateDebugQueue();

	CAN_forwardUnlock(primask);
}

/* ========================================================= */
/* CAN Forward data capture ================================ */
/* ========================================================= */
uint8_t CAN_forwardCapture(uint32_t id, uint32_t idType, const uint8_t *data, uint8_t dataLen){
	CAN_ForwardCaptureCallCount++;
	if ((data == NULL) || (dataLen > 8U) || ((idType != FDCAN_STANDARD_ID) && (idType != FDCAN_EXTENDED_ID))){
		CAN_ForwardCaptureReject++;
		return 0U;
	}

	if ((idType == FDCAN_STANDARD_ID && id > 0x7FFU) || (idType == FDCAN_EXTENDED_ID && id > 0x1FFFFFFFU)){
		CAN_ForwardCaptureReject++;
		return 0U;
	}

	if(!CAN_forwardIsAllowedID(id, idType)){
		CAN_ForwardCaptureReject++;
		return 0U;
	}
	uint32_t primask = CAN_forwardLock();

	// if queue is full
	if (queueCount >= CAN_FORWARD_QUEUE_SIZE){
		droppedFrameCount++;
		CAN_ForwardQueueFullCount++;

		CAN_forwardUpdateDebugQueue();
		CAN_forwardUnlock(primask);

//		CAN_ForwardDroppedCount = droppedFrameCount;
//		CAN_ForwardQueueCount	= queueCount;
		return 0U;
	}

	forwardQueue[queueTail].id 				= id;
	forwardQueue[queueTail].idType 			= idType;
	forwardQueue[queueTail].dlc				= dataLen;
	forwardQueue[queueTail].timestamp_ms	= HAL_GetTick();

	memset(forwardQueue[queueTail].data, 0, 8U);
	if (dataLen > 0U){
		memcpy(forwardQueue[queueTail].data, data, dataLen);
	}

	queueTail = (queueTail +1U) % CAN_FORWARD_QUEUE_SIZE;
	queueCount++;

	CAN_ForwardCaptureOK++;
	CAN_forwardUpdateDebugQueue();

	CAN_forwardUnlock(primask);
//	CAN_ForwardQueueCount = queueCount;
//	CAN_ForwardQueueHead = queueHead;
//	CAN_ForwardQueueTail = queueTail;
//	CAN_ForwardDroppedCount = droppedFrameCount;
	return 1U;
}

/* ========================================================= */
/* Build data payload ======================================= */
/* ========================================================= */
uint8_t CAN_forwardBuildPayload(
		char *output, size_t outputSize,
		uint32_t batchTimeStamp,
		uint32_t batchID,
		uint8_t *frameCount)
{
	CAN_ForwardPayloadFrameCount = 0U;
	CAN_ForwardPayloadLength = 0U;

	if (frameCount != NULL)		*frameCount = 0U;

	//Variable declaration
	CAN_forwardFrame_t frames[CAN_FORWARD_BATCH_SIZE];
	uint8_t count;
	int written;
	size_t used = 0U;

	//Parameter validation
	if((CAN_FORWARD_BATCH_SIZE == 0U) || (CAN_FORWARD_BATCH_SIZE > CAN_FORWARD_QUEUE_SIZE)){
		CAN_ForwardPayloadBuildFail++;
		return 0U;
	}
	if (frameCount != NULL) *frameCount = 0U;
	if ((output == NULL) || (frameCount == NULL) || (outputSize == 0U)){
		CAN_ForwardPayloadBuildFail++;
		return 0U;
	}

	output[0] = '\0';

	//take a frame from queue
	uint32_t primask = CAN_forwardLock();

	count = (queueCount > CAN_FORWARD_BATCH_SIZE) ? CAN_FORWARD_BATCH_SIZE : (uint8_t)queueCount;

	for (uint8_t i =0U; i < count; i++){
		uint16_t index = (queueHead + i)% CAN_FORWARD_QUEUE_SIZE;

		frames[i] = forwardQueue[index];
	}

	CAN_forwardUnlock(primask);
	if (count == 0U){
		return 0U;
	}

	//
	written = snprintf(
			output, outputSize,
			"{\"device_id\":\"%s\","
			"\"timestamp_ms\":%lu,"
			"\"batch_id\":%lu,"
			"\"can_frames\":[",
			CAN_FORWARD_DEVICE_ID, (unsigned long)batchTimeStamp,
			(unsigned long)batchID);

	if ((written < 0) || ((size_t)written >= outputSize)){
		output[0] = '\0';
		CAN_ForwardPayloadBuildFail++;
		return 0U;
	}

	used = (size_t)written;

	for (uint8_t i = 0U; i < count; i++){
		written = snprintf(
				output + used, outputSize - used,
				"%s{\"id\":\"0x%08lX\","
				"\"id_type\":\"%s\","
				"\"dlc\":%u,"
				"\"timestamp_ms\":%lu,"
				"\"data\":\"",
				(i == 0U) ? "" : ",",
				(unsigned long)frames[i].id,
				(frames[i].idType == FDCAN_EXTENDED_ID)
					? "extended" : "standard",
				(unsigned int)frames[i].dlc,
				(unsigned long)frames[i].timestamp_ms
			);

		if ((written <0) || ((size_t)written >= outputSize - used)){
			output[0] = '\0';
			CAN_ForwardPayloadBuildFail++;
			return 0U;
		}

		used += (size_t)written;

		for (uint8_t j = 0U; j < frames[i].dlc; j++){
			written = snprintf(
					output + used, outputSize - used,
					"%s%02X",
					(j == 0U) ? "" : " ",
					(unsigned int)frames[i].data[j]
			);

			if ((written < 0) || ((size_t)written >= outputSize - used)){
				output[0] = '\0';
				CAN_ForwardPayloadBuildFail++;
				return 0U;
			}

			used += (size_t)written;
		}

		written = snprintf(output + used, outputSize - used, "\"}");

		if ((written < 0) || ((size_t)written >= outputSize - used)){
			output[0] = '\0';
			CAN_ForwardPayloadBuildFail++;
			return 0U;
		}
		used += (size_t)written;
	}
	written = snprintf(output + used, outputSize - used, "]}");

	if ((written < 0) || ((size_t)written >= outputSize - used)){
		output[0] = '\0';
		CAN_ForwardPayloadBuildFail++;
		return 0U;
	}

	used += (size_t)written;

	if (used > UINT16_MAX){
		output[0] = '\0';
	    CAN_ForwardPayloadBuildFail++;
	    return 0U;
	}

	*frameCount = count;

	CAN_ForwardPayloadFrameCount 	= count;
	CAN_ForwardPayloadLength		= (uint16_t)used;
	CAN_ForwardPayloadBuildOK++;

	return 1U;
}

void CAN_forwardCommit(uint8_t frameCount)
{
	uint32_t primask = CAN_forwardLock();
	if (frameCount > queueCount)	frameCount = (uint8_t)queueCount;


	if(frameCount > 0U)
	{
		queueHead 	= (queueHead + frameCount)% CAN_FORWARD_QUEUE_SIZE;
		queueCount -= frameCount;
	}

	CAN_forwardUpdateDebugQueue();
	CAN_forwardUnlock(primask);
}

/* ========================================================= */
/* Function ================================================ */
/* ========================================================= */
uint16_t CAN_forward_getQueueCount(void){
	uint32_t primask 	= CAN_forwardLock();
	uint16_t count		= queueCount;

	CAN_forwardUnlock(primask);

	return count;
}

uint32_t CAN_forward_getDroppedCount(void){
	uint32_t primask 	= CAN_forwardLock();
	uint32_t count		= droppedFrameCount;

	CAN_forwardUnlock(primask);

	return count;
}


static void CAN_forwardUpdateDebugQueue(void)
{
    CAN_ForwardQueueCount   = queueCount;
    CAN_ForwardQueueHead    = queueHead;
    CAN_ForwardQueueTail    = queueTail;
    CAN_ForwardDroppedCount = droppedFrameCount;
}

