/*
 * app_mic_test.h
 *
 *  Created on: Jul 12, 2026
 *      Author: Cole
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "main.h"

// Handles defined in main.c
extern ETH_HandleTypeDef heth;

extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi3_rx;
extern DMA_HandleTypeDef hdma_spi6_rx;

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;
extern SAI_HandleTypeDef hsai_BlockA4;
extern DMA_HandleTypeDef hdma_sai1_a;
extern DMA_HandleTypeDef hdma_sai1_b;
extern DMA_HandleTypeDef hdma_sai4_a;

extern SPI_HandleTypeDef hspi4;

extern UART_HandleTypeDef huart3;

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern HCD_HandleTypeDef hhcd_USB_OTG_HS;

// Initialization and loop functions
HAL_StatusTypeDef app_init(void);
void app_loop(void);

// HAL callback functions
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai);

#endif /* INC_APP_MIC_TEST_H_ */
