/*
 * steeringLock_control.c
 *
 *  Created on: Sep 9, 2026
 *      Author: Think
 */

#include "IO_control/steeringLock_control.h"
#include "cfg/GPIO_config.h"

/* ========================================================= */
/* ===================== DEBUG VARIABLES =================== */
/* ========================================================= */

volatile uint8_t debug_state  = 0;
volatile uint8_t debug_button = 0;
volatile uint8_t steeringLock_status = 0;
volatile uint8_t debug_raw    = 0;

/* ========================================================= */
/* ===================== INTERNAL STATE ==================== */
/* ========================================================= */

static uint8_t steeringLock_state = 0;

static uint8_t steeringLock_sw_prev = 0;
static uint8_t steeringLock_busy = 0;
static uint8_t steeringLock_target = 0;
static uint32_t steeringLock_lastPoll = 0;

/* ========================================================= */
/* ================= INTERNAL PROTOTYPES =================== */
/* ========================================================= */

static void delay_us(uint32_t us);

static void steeringLock_SetOutput(void);
static void steeringLock_SetInput(void);

static void steeringLock_SendSync(void);
static void steeringLock_SendBit(uint8_t bit);
static void steeringLock_SendByte(uint8_t data);
static void steeringLock_SendCommand(uint16_t cmd);

static uint8_t steeringLock_WaitAck_us(uint32_t timeout_us);
static void steeringLock_WaitAckEnd(void);

static uint8_t steeringLock_ReadBit(void);
static uint8_t steeringLock_ReadByte(void);

void steeringLock_Init(void){
	/* Enable DWT for microsecond delay */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	/* Steering lock communication pin -> INPUT */
	steeringLock_SetInput();

	/* Read initial button state */
	steeringLock_sw_prev = HAL_GPIO_ReadPin(steeringLock_sw.port, steeringLock_sw.pin);

	/* Initial software state */
	steeringLock_state = 0;

	/* Not busy */
	steeringLock_busy = 0;

	/* Reset polling timer */
	steeringLock_lastPoll = HAL_GetTick();

	/* Debug */
	debug_state  = steeringLock_state;
    debug_button = steeringLock_sw_prev;
    steeringLock_status = 0;
    debug_raw    = 0;
}

/* ========================================================= */
/* ===================== PUBLIC FUNCTION =================== */
/* ========================================================= */
steeringLock_Status_t steeringLock_Lock(void){
	steeringLock_Status_t status;
	status = steeringLock_GetStatus(steeringLock_cmd_Lock);

	if (status == steeringLock_STATUS_LOCK){
		steeringLock_state = 1;
		debug_state = 1;
		steeringLock_status = 2;
	}

	return status;
}

steeringLock_Status_t steeringLock_Unlock(void){
	steeringLock_Status_t status;
	status = steeringLock_GetStatus(steeringLock_cmd_Unlock);

	if (status == steeringLock_STATUS_UNLOCK){
		steeringLock_state = 0;
		debug_state = 0;
		steeringLock_status = 1;
	}

	return status;
}

steeringLock_Status_t steeringLock_requestUnlock(void){
    steeringLock_Status_t status;

    /*
     * Jangan kirim command baru jika
     * steering lock masih bergerak.
     */
    if (steeringLock_busy != 0U)
    {
        return steeringLock_STATUS_MOVING;
    }

    status = steeringLock_Unlock();

    steeringLock_status = status;

    /*
     * Unlock sudah selesai.
     */
    if (status == steeringLock_STATUS_UNLOCK)
    {
        steeringLock_state = 0;
        steeringLock_busy = 0U;
        steeringLock_target = 1U;

        return status;
    }

    /*
     * Unlock sedang bergerak.
     */
    if (status == steeringLock_STATUS_MOVING)
    {
        steeringLock_target = 1U;
        steeringLock_busy = 1U;
        steeringLock_lastPoll = HAL_GetTick();

        return status;
    }

    /*
     * Error / status lain.
     */
    steeringLock_busy = 0U;

    return status;
}

steeringLock_Status_t steeringLock_GetStatus(uint16_t cmd){
	uint8_t raw;

	steeringLock_SendCommand(cmd); //send command
	delay_us(800);

	if (!steeringLock_WaitAck_us(60000))
		return steeringLock_STATUS_ERROR;

	steeringLock_WaitAckEnd();

	raw = steeringLock_ReadByte(); // read 1 byte raw response

	debug_raw = raw; //debug raw response

	return (steeringLock_Status_t)(raw & 0x03);
}

/* ========================================================= */
/* ======================= MAIN TASK ======================= */
/* ========================================================= */


void steeringLock_Task(void)
{
    GPIO_PinState sw_now;
    steeringLock_Status_t status;

    /* ===================================================== */
    /* READ BUTTON                                           */
    /* ===================================================== */

    sw_now = HAL_GPIO_ReadPin(
        steeringLock_sw.port,
        steeringLock_sw.pin
    );

    debug_button = sw_now;


    /* ===================================================== */
    /* BUTTON RISING EDGE                                    */
    /* ===================================================== */

    if ((sw_now == GPIO_PIN_SET) &&
        (steeringLock_sw_prev == GPIO_PIN_RESET))
    {
        /*
         * Tombol baru saja ditekan.
         *
         * Jangan menerima command baru jika
         * steering lock masih bergerak.
         */

        if (steeringLock_busy == 0)
        {
            /* ================================================= */
            /* STATE = UNLOCK                                    */
            /* → COMMAND LOCK                                   */
            /* ================================================= */

            if (steeringLock_state == 0)
            {
                status = steeringLock_Lock();

                steeringLock_status = status;

                /* LOCK sudah tercapai */

                if (status == steeringLock_STATUS_LOCK)
                {
                    steeringLock_state = 1;
                    steeringLock_busy  = 0;
                }

                /* LOCK sedang bergerak */

                else if (status == steeringLock_STATUS_MOVING)
                {
                    steeringLock_target = 0;
                    steeringLock_busy   = 1;

                    steeringLock_lastPoll = HAL_GetTick();
                }

                /* ERROR */

                else
                {
                    steeringLock_busy = 0;
                }
            }


            /* ================================================= */
            /* STATE = LOCK                                      */
            /* → COMMAND UNLOCK                                 */
            /* ================================================= */

            else
            {
                status = steeringLock_Unlock();

                steeringLock_status = status;

                /* UNLOCK sudah tercapai */

                if (status == steeringLock_STATUS_UNLOCK)
                {
                    steeringLock_state = 0;
                    steeringLock_busy  = 0;
                }

                /* UNLOCK sedang bergerak */

                else if (status == steeringLock_STATUS_MOVING)
                {
                    steeringLock_target = 1;
                    steeringLock_busy   = 1;

                    steeringLock_lastPoll = HAL_GetTick();
                }

                /* ERROR */

                else
                {
                    steeringLock_busy = 0;
                }
            }
        }
    }


    /* ===================================================== */
    /* POLLING STATUS                                        */
    /* ===================================================== */

    if (steeringLock_busy)
    {
        /*
         * Polling setiap 200 ms.
         */

        if ((HAL_GetTick() - steeringLock_lastPoll) >= 200)
        {
            steeringLock_lastPoll = HAL_GetTick();


            /* ================================================= */
            /* TARGET = LOCK                                     */
            /* ================================================= */

            if (steeringLock_target == 0)
            {
                status = steeringLock_Lock();

                steeringLock_status = status;


                /* LOCK selesai */

                if (status == steeringLock_STATUS_LOCK)
                {
                    steeringLock_state = 1;
                    steeringLock_busy  = 0;
                }


                /* Masih bergerak */

                else if (status == steeringLock_STATUS_MOVING)
                {
                    /*
                     * Tetap busy.
                     */
                }


                /* ERROR */

                else
                {
                    steeringLock_busy = 0;
                }
            }


            /* ================================================= */
            /* TARGET = UNLOCK                                   */
            /* ================================================= */

            else
            {
                status = steeringLock_Unlock();

                steeringLock_status = status;


                /* UNLOCK selesai */

                if (status == steeringLock_STATUS_UNLOCK)
                {
                    steeringLock_state = 0;
                    steeringLock_busy  = 0;
                }


                /* Masih bergerak */

                else if (status == steeringLock_STATUS_MOVING)
                {
                    /*
                     * Tetap busy.
                     */
                }


                /* ERROR */

                else
                {
                    steeringLock_busy = 0;
                }
            }
        }
    }


    /* ===================================================== */
    /* SAVE BUTTON STATE                                     */
    /* ===================================================== */

    steeringLock_sw_prev = sw_now;


    /* ===================================================== */
    /* DEBUG                                                 */
    /* ===================================================== */

    debug_state = steeringLock_state;
}

/* ========================================================= */
/* =================== INTERNAL FUNCTIONS ================== */
/* ========================================================= */

static void delay_us(uint32_t us){
	uint32_t cycles = (SystemCoreClock / 1000000) * us;
	uint32_t start 	= DWT->CYCCNT;
	while ((DWT->CYCCNT - start) < cycles);
}

static void steeringLock_SetOutput(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = steeringLock.pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(steeringLock.port, &GPIO_InitStruct);
}

static void steeringLock_SetInput(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = steeringLock.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(steeringLock.port, &GPIO_InitStruct);
}

static void steeringLock_SendSync(void){
	HAL_GPIO_WritePin(steeringLock.port, steeringLock.pin, GPIO_PIN_RESET);
	delay_us(8000);
}

static void steeringLock_SendBit(uint8_t bit){
	HAL_GPIO_WritePin(steeringLock.port, steeringLock.pin, GPIO_PIN_SET);

	 if (bit)
		 delay_us(3000);
	 else
		 delay_us(1000);

	HAL_GPIO_WritePin(steeringLock.port, steeringLock.pin, GPIO_PIN_RESET);

 	 if (bit)
 		 delay_us(1000);
 	 else
 		 delay_us(3000);

}

static void steeringLock_SendByte(uint8_t data){
	for (int i = 7; i >= 0; i--){
        steeringLock_SendBit((data >> i) & 0x01);
    }
}

static void steeringLock_SendCommand(uint16_t cmd){
	steeringLock_SetOutput();

	steeringLock_SendSync();
	steeringLock_SendByte((cmd >> 8) & 0xFF);
	steeringLock_SendByte(cmd & 0xFF);
	HAL_GPIO_WritePin(steeringLock.port, steeringLock.pin, GPIO_PIN_SET);

	steeringLock_SetInput();
}

static uint8_t steeringLock_WaitAck_us(uint32_t timeout_us){
	 while (timeout_us--){
        if (HAL_GPIO_ReadPin(steeringLock.port, steeringLock.pin) == GPIO_PIN_RESET){
            return 1;
        }
        delay_us(1);
	 }
	 return 0;
}

static void steeringLock_WaitAckEnd(void){
	while (HAL_GPIO_ReadPin(steeringLock.port, steeringLock.pin) == GPIO_PIN_RESET){
        delay_us(1);
    }
}

static uint8_t steeringLock_ReadBit(void){
	uint32_t t_high = 0;
    uint32_t t_low  = 0;

    /*HIGH*/
    while (HAL_GPIO_ReadPin(steeringLock.port, steeringLock.pin) == GPIO_PIN_SET){
        t_high++;
	    delay_us(1);
    }

	/*LOW*/
    while (HAL_GPIO_ReadPin(steeringLock.port, steeringLock.pin) == GPIO_PIN_RESET){
        t_low++;
        delay_us(1);
    }
	    /*
     * HIGH lebih lama → bit 1
     * LOW lebih lama  → bit 0
     */

    return (t_high > t_low) ? 1 : 0;
}
static uint8_t steeringLock_ReadByte(void){
	uint8_t data = 0;
	for (int i = 0; i<8; i++){
		data <<= 1;
		data |= steeringLock_ReadBit();
	}
	return data;
}
