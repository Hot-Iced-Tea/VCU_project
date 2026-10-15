/*
 * GPIO.c
 *
 *  Created on: Jul 27, 2026
 *      Author: fahmif_imoto
 */
#include "cfg/GPIO.h"

#define Rbrake_sw 		IN1_Pin
#define sideStand_sw 	IN3_Pin

static const uint16_t OUT_PINS[4] = {OUT1_Pin, OUT2_Pin, OUT3_Pin, OUT4_Pin};


uint8_t btn_status = 0;
char status_text[9] = "UNLOCKED";

//static const uint16_t OUT_PINS[4] = {
//		{GPIOC, OUT1_Pin},
//		{GPIOC, OUT2_Pin},
//		{GPIOC, OUT3_Pin},
//		{OUT4_GPIO_Port, OUT4_Pin}
//};

void OUT_SET(uint16_t pin_set){
	HAL_GPIO_WritePin(GPIOC, pin_set, GPIO_PIN_SET);
	HAL_GPIO_WritePin(OUT4_GPIO_Port, pin_set, GPIO_PIN_SET);
}

void OUT_RESET(uint16_t pin_reset){
	HAL_GPIO_WritePin(GPIOC, pin_reset, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(OUT4_GPIO_Port, pin_reset, GPIO_PIN_RESET);
}

void OUT_TOGGLE(uint16_t pin_toggle){
	HAL_GPIO_TogglePin(GPIOC, pin_toggle);
	HAL_GPIO_TogglePin(OUT4_GPIO_Port, pin_toggle);
}

void GPIO_delay (uint32_t delay_ms){
	HAL_Delay(delay_ms);
}

void countBlink() {

}

//tes pin output
void Blinky() {
//	OUT_SET(OUT1_Pin);
//	GPIO_delay(100);
//	OUT_RESET(OUT1_Pin);
//	GPIO_delay(100);
	for (int i = 0; i<4; i++){
		OUT_RESET((uint16_t)(OUT1_Pin | OUT2_Pin | OUT3_Pin | OUT4_Pin));
		OUT_SET(OUT_PINS[i]);
		GPIO_delay(50);
	}
}

//tes pin input
void locked_status(){
	if (HAL_GPIO_ReadPin(IN3_GPIO_Port, sideStand_sw) == GPIO_PIN_SET){
		btn_status = 1;
		memset(status_text, 0, sizeof(status_text));
		strcpy(status_text, "UNLOCKED");
		Blinky();
	}
	else {
		btn_status = 0;
		memset(status_text, 0, sizeof(status_text));
		strcpy(status_text, "LOCKED");
		OUT_RESET((uint16_t)(OUT1_Pin | OUT2_Pin | OUT3_Pin | OUT4_Pin));
	}
}

