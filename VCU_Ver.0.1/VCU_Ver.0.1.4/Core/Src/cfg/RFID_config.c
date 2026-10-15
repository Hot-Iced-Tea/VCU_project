/*
 * RFID_config.c
 *
 *  Created on: Sep 22, 2026
 *      Author: fahmif_imoto
 */

#include "RFID_config.h"

const GPIO_Config RFID_CS 	= {OUT_SPI_CS_GPIO_Port, OUT_SPI_CS_Pin};
const GPIO_Config RFID_RST 	= {OUT_SPI_RST_GPIO_Port, OUT_SPI_RST_Pin};

void RFID_config_init(void){
	output_setState(&RFID_CS, GPIO_PIN_SET);
	output_setState(&RFID_RST, GPIO_PIN_SET);
}

