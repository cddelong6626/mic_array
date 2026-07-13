/*
 * app.h
 *
 *  Created on: Jul 12, 2026
 *      Author: Cole
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "main.h"

// Handles defined in main.c
extern ETH_HandleTypeDef heth;

extern I2S_HandleTypeDef hi2s2;
extern I2S_HandleTypeDef hi2s3;
extern I2S_HandleTypeDef hi2s6;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi3_rx;
extern DMA_HandleTypeDef hdma_spi6_rx;

extern SPI_HandleTypeDef hspi4;

extern UART_HandleTypeDef huart3;

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern HCD_HandleTypeDef hhcd_USB_OTG_HS;

// Initialization and loop functions
HAL_StatusTypeDef app_init(void);
void app_loop(void);

// HAL callback functions
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s);

#endif /* INC_APP_H_ */
