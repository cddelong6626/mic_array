/*
 * sk9822.c
 *
 *  Created on: Jul 11, 2026
 *      Author: Cole Delong
 */

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "sk9822.h"

// Brightness byte layout: top 3 bits must be 1 (per SK9822 protocol), bottom 5 bits are brightness (0-31)
#define SK9822_BRIGHTNESS_MASK 0b00011111
#define SK9822_PADDING_BITS    0b11100000

typedef struct __attribute__((__packed__))
{
    uint8_t brightness; // top 3 bits: padding (always 1), bottom 5 bits: brightness (0-31)
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} led_frame_t;

typedef struct
{
    uint32_t start_frame;
    led_frame_t led_frames[N_LEDS];
    uint32_t end_frame;
} data_frame_t;

data_frame_t _df;
SPI_HandleTypeDef *_h_spi;

HAL_StatusTypeDef sk9822_init(SPI_HandleTypeDef *h_spi)
{
    // Store handle to LED SPI interface
    _h_spi = h_spi;

    // Start Frame
    _df.start_frame = 0;

    // LED Frames
    for (int i = 0; i < N_LEDS; i++)
    {
        // Arbitrary default: initialize all LEDs to be white with zero brightness
        _df.led_frames[i].brightness = SK9822_PADDING_BITS
                | (0x00 & SK9822_BRIGHTNESS_MASK);
        _df.led_frames[i].blue = 255;
        _df.led_frames[i].green = 255;
        _df.led_frames[i].red = 255;
    }

    // End Frame
    _df.end_frame = 0xFFFFFFFF;

    // Send update to strip
    return sk9822_send_update();
}

HAL_StatusTypeDef sk9822_send_update(void)
{
    // This is a very short transmission so a blocking SPI call is acceptable
    return HAL_SPI_Transmit(_h_spi, (uint8_t*) &_df, sizeof(_df), 100);
}

void sk9822_edit_led(int i_led, int red, int green, int blue, int brightness)
{
    // -1 means don't edit that property
    if (red != -1)
        _df.led_frames[i_led].red = red;
    if (green != -1)
        _df.led_frames[i_led].green = green;
    if (blue != -1)
        _df.led_frames[i_led].blue = blue;
    if (brightness != -1)
    {
        // The brightness byte is three bits of padding followed by the 5-bit brightness value
        _df.led_frames[i_led].brightness = SK9822_PADDING_BITS
                | (brightness & SK9822_BRIGHTNESS_MASK);
    }
}
