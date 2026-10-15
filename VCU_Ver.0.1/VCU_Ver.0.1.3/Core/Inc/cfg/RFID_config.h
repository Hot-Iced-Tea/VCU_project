/*
 * RFID_config.h
 *
 *  Created on: Sep 22, 2026
 *      Author: fahmif_imoto
 */

#ifndef INC_CFG_RFID_CONFIG_H_
#define INC_CFG_RFID_CONFIG_H_

#include "main.h"
#include "GPIO_config.h"
#include "IO_control/output_ctrl.h"

#define RFID_SPI_HANDLE hspi2

extern const GPIO_Config RFID_CS;
extern const GPIO_Config RFID_RST;

void RFID_config_init(void);

#endif /* INC_CFG_RFID_CONFIG_H_ */
