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

// 16-bits rather than 24-bits per sample since mic SNR only gives 10-11 bits anyway
int16_t sai1a_rx_buf[BUF_LEN] __attribute__((section(".dma_buffer")));      // DMA buffer: cache disabled
int16_t sai1b_rx_buf[BUF_LEN] __attribute__((section(".dma_buffer")));
int16_t sai4a_rx_buf[BUF_LEN] __attribute__((section(".bdma_buffer")));     // BDMA buffer: cache disabled, D3 ram
int16_t sai4b_rx_buf[BUF_LEN] __attribute__((section(".bdma_buffer")));

// 8 channels: sai1_BlockA(0,1)  sai1_BlockB(2,3)  sai4_BlockA(4,5)  sai4_BlockB(6,7)
float32_t audio_buf[8][SAMPLES_PER_HALF_PER_CHAN];

volatile char flag_sai1a_half_filled = 0;
volatile char flag_sai1a_full_filled = 0;
volatile char flag_sai1b_half_filled = 0;
volatile char flag_sai1b_full_filled = 0;
volatile char flag_sai4a_half_filled = 0;
volatile char flag_sai4a_full_filled = 0;
volatile char flag_sai4b_half_filled = 0;
volatile char flag_sai4b_full_filled = 0;

int i_led;
volatile int x = 0;

volatile int x1a = 0;
volatile int x1b = 0;
volatile int x4a = 0;
volatile int x4b = 0;



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

    // Slave block(s) must be armed before the master starts clocking
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)sai1b_rx_buf, BUF_LEN);
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockA4, (uint8_t *)sai4a_rx_buf, BUF_LEN);
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockB4, (uint8_t *)sai4b_rx_buf, BUF_LEN);
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)sai1a_rx_buf, BUF_LEN);

    i_led = 0;

    return ret_status;
}

void app_loop(void)
{
    HAL_StatusTypeDef ret_status;
    while (1)
    {
        if (flag_sai1a_half_filled)
        {
            deinterleave_and_convert(&sai1a_rx_buf[0], 0, 1);
            flag_sai1a_half_filled = 0;
        }
        if (flag_sai1a_full_filled)
        {
            deinterleave_and_convert(&sai1a_rx_buf[SAMPLES_PER_HALF], 0, 1);
            flag_sai1a_full_filled = 0;
        }
        if (flag_sai1b_half_filled)
        {
            deinterleave_and_convert(&sai1b_rx_buf[0], 2, 3);
            flag_sai1b_half_filled = 0;
        }
        if (flag_sai1b_full_filled)
        {
            deinterleave_and_convert(&sai1b_rx_buf[SAMPLES_PER_HALF], 2, 3);
            flag_sai1b_full_filled = 0;
        }
        if (flag_sai4a_half_filled)
        {
            deinterleave_and_convert(&sai4a_rx_buf[0], 4, 5);
            flag_sai4a_half_filled = 0;
        }
        if (flag_sai4a_full_filled)
        {
            deinterleave_and_convert(&sai4a_rx_buf[SAMPLES_PER_HALF], 4, 5);
            flag_sai4a_full_filled = 0;
        }
        if (flag_sai4b_half_filled)
        {
            deinterleave_and_convert(&sai4b_rx_buf[0], 6, 7);
            flag_sai4b_half_filled = 0;
        }
        if (flag_sai4b_full_filled)
        {
            deinterleave_and_convert(&sai4b_rx_buf[SAMPLES_PER_HALF], 6, 7);
            flag_sai4b_full_filled = 0;
        }

        printf("s1a:%6d s1b:%6d s4a:%6d s4b:%6d | c: %6d %6d %6d %6d\n\r",
               sai1a_rx_buf[0], sai1b_rx_buf[0], sai4a_rx_buf[0], sai4b_rx_buf[1],
               x1a, x1b, x4a, x4b);
        HAL_Delay(100);
    }

}


// HAL callback functions
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == hsai_BlockA1.Instance)
    {
        flag_sai1a_half_filled = 1;
        x1a++;
    }
    else if (hsai->Instance == hsai_BlockB1.Instance)
    {
        flag_sai1b_half_filled = 1;
        x1b++;
    }
    else if (hsai->Instance == hsai_BlockA4.Instance)
    {
        flag_sai4a_half_filled = 1;
        x4a++;
    }
    else if (hsai->Instance == hsai_BlockB4.Instance)
    {
        flag_sai4b_half_filled = 1;
        x4b++;
    }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == hsai_BlockA1.Instance)
    {
        flag_sai1a_full_filled = 1;
    }
    else if (hsai->Instance == hsai_BlockB1.Instance)
    {
        flag_sai1b_full_filled = 1;
    }
    else if (hsai->Instance == hsai_BlockA4.Instance)
    {
        flag_sai4a_full_filled = 1;
    }
    else if (hsai->Instance == hsai_BlockB4.Instance)
    {
        flag_sai4b_full_filled = 1;
    }
}


// Override system call to route "printf" calls over the ST-Link to the PC
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
