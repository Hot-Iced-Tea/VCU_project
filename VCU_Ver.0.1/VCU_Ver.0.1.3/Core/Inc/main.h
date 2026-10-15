/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OUT3_Pin GPIO_PIN_13
#define OUT3_GPIO_Port GPIOC
#define OUT2_Pin GPIO_PIN_14
#define OUT2_GPIO_Port GPIOC
#define OUT1_Pin GPIO_PIN_15
#define OUT1_GPIO_Port GPIOC
#define LTE_RX_Pin GPIO_PIN_1
#define LTE_RX_GPIO_Port GPIOA
#define LTE_TX_Pin GPIO_PIN_2
#define LTE_TX_GPIO_Port GPIOA
#define LTE_ON_Pin GPIO_PIN_5
#define LTE_ON_GPIO_Port GPIOA
#define PWM1_Pin GPIO_PIN_1
#define PWM1_GPIO_Port GPIOB
#define PWM2_Pin GPIO_PIN_2
#define PWM2_GPIO_Port GPIOB
#define OUT_SPI_RST_Pin GPIO_PIN_10
#define OUT_SPI_RST_GPIO_Port GPIOB
#define OUT_SPI_CS_Pin GPIO_PIN_12
#define OUT_SPI_CS_GPIO_Port GPIOB
#define RFID_SCK_Pin GPIO_PIN_13
#define RFID_SCK_GPIO_Port GPIOB
#define RFID_MISO_Pin GPIO_PIN_14
#define RFID_MISO_GPIO_Port GPIOB
#define RFID_MOSI_Pin GPIO_PIN_15
#define RFID_MOSI_GPIO_Port GPIOB
#define USART2_OUT_Pin GPIO_PIN_8
#define USART2_OUT_GPIO_Port GPIOC
#define CAN_TX_Pin GPIO_PIN_9
#define CAN_TX_GPIO_Port GPIOC
#define CAN_RX_Pin GPIO_PIN_8
#define CAN_RX_GPIO_Port GPIOA
#define CAN_STB_Pin GPIO_PIN_11
#define CAN_STB_GPIO_Port GPIOA
#define IN4_Pin GPIO_PIN_4
#define IN4_GPIO_Port GPIOB
#define IN3_Pin GPIO_PIN_5
#define IN3_GPIO_Port GPIOB
#define IN2_Pin GPIO_PIN_6
#define IN2_GPIO_Port GPIOB
#define IN1_Pin GPIO_PIN_7
#define IN1_GPIO_Port GPIOB
#define OUT4_Pin GPIO_PIN_8
#define OUT4_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
