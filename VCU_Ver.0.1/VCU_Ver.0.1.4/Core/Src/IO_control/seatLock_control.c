/*
 * seatLock_control.c
 *
 *  Created on: Sep 10, 2026
 *      Author: Think
 */

#include "IO_control/seatLock_control.h"
#include "cfg/GPIO_config.h"
#include "IO_control/output_ctrl.h"
#include "IO_control/input_ctrl.h"

#define SEATLOCK_SHORT_US 				1000U
#define SEATLOCK_LONG_US				3000U

#define SEATLOCK_COMMAND_REPETITIONS 	   2U

#define SEATLOCK_DEBOUNCE_MS                 50U
#define SEATLOCK_RETRIGGER_LOCKOUT_MS       500U

#define SEATLOCK_AUTO_REPEAT 			   1U
#define SEATLOCK_MECHANICAL_WAIT_MS 	 500U
#define SEATLOCK_REPEAT_PERIOD_MS 		5000U

//volatile GPIO_PinState debug_seatLock_input = GPIO_PIN_RESET;
//volatile GPIO_PinState debug_seatLock_state = GPIO_PIN_RESET;
//volatile GPIO_PinState debug_seatLock_prev = GPIO_PIN_RESET;
//volatile uint8_t debug_seatLock_trigger = 0U;

volatile GPIO_PinState debug_seatLock_raw = GPIO_PIN_RESET;
volatile uint32_t debug_seatLock_trigger_count = 0U;

static void delay_us(uint32_t us);
//static void seatLock_setOutput(void);
static void seatLock_write(uint8_t high);
static void seatLock_sendBit(uint8_t bit);
//static void seatLock_sendByte(uint8_t);

#define SEATLOCK_BUTTON_ACTIVE_STATE GPIO_PIN_SET

static GPIO_PinState seatLock_raw_previous;
static GPIO_PinState seatLock_debounced_state;
static uint32_t seatLock_last_raw_change_ms;
static uint32_t seatLock_last_trigger_ms;

static void delay_us(uint32_t us){
	uint32_t cycles = (SystemCoreClock / 1000000U) * us;
	uint32_t start = DWT->CYCCNT;
	while ((DWT->CYCCNT - start) < cycles);
}

static void seatLock_write(uint8_t high){
	output_setState(&seatLock, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void seatLock_sendBit(uint8_t bit){
	if (bit != 0U){
		seatLock_write(1U);
		delay_us(SEATLOCK_LONG_US);

		seatLock_write(0U);
		delay_us(SEATLOCK_SHORT_US);
	}
	else {
		seatLock_write(1U);
		delay_us(SEATLOCK_SHORT_US);

		seatLock_write(0U);
		delay_us(SEATLOCK_LONG_US);
	}
}

void seatLock_Init(void){
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0U;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	seatLock_write(0U);

	seatLock_raw_previous = input_getState(&seatLock_sw);
	seatLock_debounced_state = seatLock_raw_previous;
    seatLock_last_raw_change_ms = HAL_GetTick();
    seatLock_last_trigger_ms = HAL_GetTick() - SEATLOCK_RETRIGGER_LOCKOUT_MS;
}

void seatLock_triggerOpen(void){
//	seatLock_setOutput();
	uint8_t repeat;
	uint8_t bit;

	seatLock_write(0U);

	delay_us(SEATLOCK_SHORT_US);

	for (repeat = 0U; repeat < SEATLOCK_COMMAND_REPETITIONS; ++repeat){
		for (bit = 0U; bit < 8U; ++bit){
			seatLock_sendBit((uint8_t)((seatLock_cmd_Trig >> (7U - bit)) & 1U));
		}
	}

	seatLock_write(0U);
}

void seatLock_task(void){
	const uint32_t now = HAL_GetTick();
    const GPIO_PinState raw = input_getState(&seatLock_sw);
    debug_seatLock_raw = raw;

	    /* First stage: record every raw input transition. */
    if (raw != seatLock_raw_previous){
    	seatLock_raw_previous = raw;
    	seatLock_last_raw_change_ms = now;
    }

    /* Second stage: accept a state only after it has been stable for 50 ms. */
        if (((uint32_t)(now - seatLock_last_raw_change_ms) >= SEATLOCK_DEBOUNCE_MS) &&
            (seatLock_debounced_state != seatLock_raw_previous)){
        	const GPIO_PinState previous = seatLock_debounced_state;
        	seatLock_debounced_state = seatLock_raw_previous;

        	if ((seatLock_debounced_state == SEATLOCK_BUTTON_ACTIVE_STATE) &&
        	    (previous != SEATLOCK_BUTTON_ACTIVE_STATE) &&
        	    ((uint32_t)(now - seatLock_last_trigger_ms) >= SEATLOCK_RETRIGGER_LOCKOUT_MS)){
        		seatLock_last_trigger_ms = now;
        		debug_seatLock_trigger_count++;
        		seatLock_triggerOpen();
        	}
        }

}
