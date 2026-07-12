/*
 * sk9822.h
 *
 *  Created on: Jul 11, 2026
 *      Author: Cole Delong
 */

#ifndef INC_SK9822_H_
#define INC_SK9822_H_

#include <stdint.h>
#include "stm32h7xx_hal.h"

// Needed at compile time
#define N_LEDS 12

/**
 * @brief  Initializes the SK9822 LED driver. Stores the SPI handle used for
 *         communication, sets all LEDs to full brightness/white, and sends
 *         that initial frame out over SPI.
 *
 * @param  h_spi Pointer to the initialized SPI handle connected to the LED array
 * @retval HAL_StatusTypeDef HAL status returned by the initial SPI transmit
 */
HAL_StatusTypeDef sk9822_init(SPI_HandleTypeDef* h_spi);


/**
 * @brief  Transmits the current LED frame buffer over SPI, updating the
 *         physical LED array to match the values last set via sk9822_edit_led().
 *
 * @param  None
 * @retval HAL_StatusTypeDef HAL status returned by HAL_SPI_Transmit
 */
HAL_StatusTypeDef sk9822_send_update(void);

/**
 * @brief  Edits the color and/or brightness of one LED in the local frame
 *         buffer. Does not send the update to the LEDs; call
 *         sk9822_send_update() afterward to apply the change.
 *
 * @param  i_led      Index of the LED to edit (0 to N_LEDS - 1)
 * @param  red        8-bit RGB red value (0-255), or -1 to leave unchanged
 * @param  green      8-bit RGB green value (0-255), or -1 to leave unchanged
 * @param  blue       8-bit RGB blue value (0-255), or -1 to leave unchanged
 * @param  brightness 5-bit brightness value (0-31), or -1 to leave unchanged
 * @retval None
 */
void sk9822_edit_led(int i_led, int red, int green, int blue, int brightness);

#endif /* INC_SK9822_H_ */
