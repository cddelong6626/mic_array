/*
 * app.c
 *
 *  Created on: Jul 12, 2026
 *      Author: Cole
 */

#include <stdint.h>
#include <stdio.h>
#include "arm_math.h"
#include "app.h"
#include "sk9822.h"


// Double buffer made using circular buffer
#define SAMPLES_PER_HALF_PER_CHAN   128
#define SAMPLES_PER_HALF            (SAMPLES_PER_HALF_PER_CHAN * 2)
#define BUF_LEN                     (SAMPLES_PER_HALF * 2)

// Half-word rather than 24-bits per sample since mic SNR only gives 10-11 bits anyway
int16_t i2s2_rx_buf[BUF_LEN];
int16_t i2s3_rx_buf[BUF_LEN];
int16_t i2s6_rx_buf[BUF_LEN];

float32_t audio_buf[6][SAMPLES_PER_HALF_PER_CHAN];

volatile char flag_i2s2_half_filled = 0;
volatile char flag_i2s2_full_filled = 0;

int i_led;


void deinterleave_and_convert(const int16_t *rx_buf, const uint8_t i_rchannel, const uint8_t i_lchannel)
{
    for (uint32_t i = 0; i < SAMPLES_PER_HALF_PER_CHAN; ++i)
    {
        int16_t left_rx  = rx_buf[i * 2 + 0];
        int16_t right_rx = rx_buf[i * 2 + 1];

        audio_buf[i_lchannel][i] = (float32_t)left_rx  / 32768.0f;
        audio_buf[i_rchannel][i] = (float32_t)right_rx / 32768.0f;
    }
}



// Initialization and loop functions
HAL_StatusTypeDef app_init(void)
{
    HAL_StatusTypeDef ret_status = HAL_OK;

    // Initialize LED array
    ret_status |= sk9822_init(&hspi4);

    // Start DMA
    HAL_I2S_Receive_DMA(&hi2s2, i2s2_rx_buf, BUF_LEN);

    i_led = 0;

    return ret_status;
}

void app_loop(void)
{
//    HAL_StatusTypeDef ret_status;
    while (1)
    {
        if (flag_i2s2_half_filled)
        {
            deinterleave_and_convert(&i2s2_rx_buf[0], 0, 1);
            flag_i2s2_half_filled = 0;

        }
        if (flag_i2s2_full_filled)
        {
            deinterleave_and_convert(&i2s2_rx_buf[SAMPLES_PER_HALF], 0, 1);
            flag_i2s2_full_filled = 0;
        }

        printf("%d\t%d\t%d\t%d\n\r", i2s2_rx_buf[0], i2s2_rx_buf[1], i2s2_rx_buf[2], i2s2_rx_buf[3]);
        HAL_Delay(100);

//        if (flag)
//        {
//            ret_status = HAL_OK;
//
//            HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);
//
//            sk9822_edit_led(i_led, -1, -1, -1, 0);
//            sk9822_edit_led((i_led + 4) % 12, -1, -1, -1, 0);
//            sk9822_edit_led((i_led + 8) % 12, -1, -1, -1, 0);
//
//            ++i_led;
//            if (i_led >= 12)
//                i_led = 0;
//
//            sk9822_edit_led(i_led, 255, 0, 0, 31);
//            sk9822_edit_led((i_led + 4) % 12, 0, 255, 0, 31);
//            sk9822_edit_led((i_led + 8) % 12, 0, 0, 255, 31);
//
//            ret_status |= sk9822_send_update();
//
//            for (int i = 0; i < 10000000 / 4; i++)
//            {
//                x++;
//            }
//
//        }
    }

}


// HAL callback functions
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == hi2s2.Instance)
    {
        flag_i2s2_half_filled = 1;
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == hi2s2.Instance)
    {
        flag_i2s2_full_filled = 1;
    }
}


// Override system call to route "printf" calls over the ST-Link to the PC
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

