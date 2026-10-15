/*
 * CAN_LTE_dataForward.h
 *
 *  Created on: Oct 9, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_CAN_PROTOCOL_CAN_LTE_DATAFORWARD_H_
#define INC_CAN_PROTOCOL_CAN_LTE_DATAFORWARD_H_

#include <stdint.h>
#include <stddef.h>

/* ========================================================= */
/* QUEUE CAPACITY & BATCH SIZE PER PAYLOAD ================= */
/* ========================================================= */
#define CAN_FORWARD_QUEUE_SIZE 	64U
#define CAN_FORWARD_BATCH_SIZE	10U

extern volatile uint32_t CAN_ForwardCaptureCallCount;
extern volatile uint32_t CAN_ForwardCaptureOK;
extern volatile uint32_t CAN_ForwardCaptureReject;
extern volatile uint32_t CAN_ForwardQueueFullCount;

extern volatile uint16_t CAN_ForwardQueueCount;
extern volatile uint16_t CAN_ForwardQueueHead;
extern volatile uint16_t CAN_ForwardQueueTail;
extern volatile uint32_t CAN_ForwardDroppedCount;

extern volatile uint32_t CAN_ForwardPayloadBuildOK;
extern volatile uint32_t CAN_ForwardPayloadBuildFail;
extern volatile uint8_t  CAN_ForwardPayloadFrameCount;
extern volatile uint16_t CAN_ForwardPayloadLength;


#if (CAN_FORWARD_QUEUE_SIZE == 0U)
#error "CAN_FORWARD_QUEUE_SIZE must be greater than zero"
#endif

#if (CAN_FORWARD_BATCH_SIZE == 0U)
#error "CAN_FORWARD_BATCH_SIZE must be greater than zero"
#endif

#if (CAN_FORWARD_BATCH_SIZE > CAN_FORWARD_QUEUE_SIZE)
#error "CAN_FORWARD_BATCH_SIZE cannot exceed queue size"
#endif

#if (CAN_FORWARD_BATCH_SIZE > UINT8_MAX)
#error "CAN_FORWARD_BATCH_SIZE exceeds frameCount capacity"
#endif

/* ========================================================= */
/* CAN Frame Structure ===================================== */
/* ========================================================= */
typedef struct{
	uint32_t id;
	uint32_t timestamp_ms;
	uint32_t idType;
	uint8_t	 dlc;
	uint8_t	 data[8];
}CAN_forwardFrame_t;

/* ========================================================= */
/* Initialization ========================================== */
/* ========================================================= */
void CAN_forward_Init(void);

//CAN frame that input to queue
uint8_t CAN_forwardCapture(uint32_t id, uint32_t idType, const uint8_t *data, uint8_t datalen);

uint8_t CAN_forwardBuildPayload(char *output, size_t outputSize, uint32_t batchTimeStamp, uint32_t batchID, uint8_t *frameCount);

void CAN_forwardCommit(uint8_t frameCount);

/* ========================================================= */
/* Function ============================================== */
/* ========================================================= */
uint16_t CAN_forward_getQueueCount(void);
uint32_t CAN_forward_getDroppedCount(void);

#endif /* INC_CAN_PROTOCOL_CAN_LTE_DATAFORWARD_H_ */
