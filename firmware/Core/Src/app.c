/*
 * app.c
 *
 *  Created on: Jul 12, 2026
 *      Author: Cole Delong
 */

#include <stdint.h>
#include <stdio.h>
#include "arm_math.h"
#include "app.h"
#include "sk9822.h"


// Double buffer made using circular buffer
#define SAMPLES_PER_HALF_PER_CHAN   256
#define SAMPLES_PER_HALF            (SAMPLES_PER_HALF_PER_CHAN * 2)
#define BUF_LEN                     (SAMPLES_PER_HALF * 2)

// 16-bits rather than 24-bits per sample since mic SNR only gives 10-11 bits anyway
int16_t sai1a_rx_buf[BUF_LEN] __attribute__((section(".dma_buffer")));      // DMA buffer: cache disabled
int16_t sai1b_rx_buf[BUF_LEN] __attribute__((section(".dma_buffer")));
int16_t sai4a_rx_buf[BUF_LEN] __attribute__((section(".bdma_buffer")));     // BDMA buffer: cache disabled, D3 ram

// 6 channels: sai1_BlockA(0,1)  sai1_BlockB(2,3)  sai4_BlockA(4,5)
float32_t audio_buf[2][6][SAMPLES_PER_HALF_PER_CHAN];

volatile uint8_t flag_sai1a_half0_filled = 0;
volatile uint8_t flag_sai1b_half0_filled = 0;
volatile uint8_t flag_sai4a_half0_filled = 0;
volatile uint8_t flag_sai1a_half1_filled = 0;
volatile uint8_t flag_sai1b_half1_filled = 0;
volatile uint8_t flag_sai4a_half1_filled = 0;

uint8_t flag_chan01_half0_ready = 0;
uint8_t flag_chan23_half0_ready = 0;
uint8_t flag_chan45_half0_ready = 0;
uint8_t flag_chan01_half1_ready = 0;
uint8_t flag_chan23_half1_ready = 0;
uint8_t flag_chan45_half1_ready = 0;


// Function prototypes
void deinterleave_and_convert(
        float32_t output_buf[6][SAMPLES_PER_HALF_PER_CHAN],
        const int16_t *rx_buf,
        const uint8_t i_rchannel,
        const uint8_t i_lchannel);
void process_samples(float32_t output_buf[6][SAMPLES_PER_HALF_PER_CHAN]);


// Initialization
HAL_StatusTypeDef app_init(void)
{
    HAL_StatusTypeDef ret_status = HAL_OK;

    // Initialize LED array
    ret_status |= sk9822_init(&hspi4);

    // Slave block(s) must be armed before the master starts clocking
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)sai1b_rx_buf, BUF_LEN);
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockA4, (uint8_t *)sai4a_rx_buf, BUF_LEN);
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)sai1a_rx_buf, BUF_LEN);

    return ret_status;
}

// Main loop
void app_loop(void)
{
    while (1)
    {
        // Transfer data from interleaved DMA buffers to multi-channel buffer
        if (flag_sai1a_half0_filled)
        {
            flag_sai1a_half0_filled = 0;
            deinterleave_and_convert(audio_buf[0], &sai1a_rx_buf[0], 0, 1);
            flag_chan01_half0_ready = 1;
        }

        if (flag_sai1b_half0_filled)
        {
            flag_sai1b_half0_filled = 0;
            deinterleave_and_convert(audio_buf[0], &sai1b_rx_buf[0], 2, 3);
            flag_chan23_half0_ready = 1;
        }

        if (flag_sai4a_half0_filled)
        {
            flag_sai4a_half0_filled = 0;
            deinterleave_and_convert(audio_buf[0], &sai4a_rx_buf[0], 4, 5);
            flag_chan45_half0_ready = 1;
        }

        if (flag_sai1a_half1_filled)
        {
            flag_sai1a_half1_filled = 0;
            deinterleave_and_convert(audio_buf[1], &sai1a_rx_buf[SAMPLES_PER_HALF], 0, 1);
            flag_chan01_half1_ready = 1;
        }

        if (flag_sai1b_half1_filled)
        {
            flag_sai1b_half1_filled = 0;
            deinterleave_and_convert(audio_buf[1], &sai1b_rx_buf[SAMPLES_PER_HALF], 2, 3);
            flag_chan23_half1_ready = 1;
        }

        if (flag_sai4a_half1_filled)
        {
            flag_sai4a_half1_filled = 0;
            deinterleave_and_convert(audio_buf[1], &sai4a_rx_buf[SAMPLES_PER_HALF], 4, 5);
            flag_sai4a_half1_filled = 1;
        }

        // When half of the buffer is filled: process that half
        if (flag_chan01_half0_ready && flag_chan23_half0_ready && flag_chan45_half0_ready)
        {
            flag_chan01_half0_ready = 0;
            flag_chan23_half0_ready = 0;
            flag_chan45_half0_ready = 0;

            process_samples(audio_buf[0]);
        }
        if (flag_chan01_half1_ready && flag_chan23_half1_ready && flag_chan45_half1_ready)
        {
            flag_chan01_half1_ready = 0;
            flag_chan23_half1_ready = 0;
            flag_chan45_half1_ready = 0;

            process_samples(audio_buf[1]);
        }
    }
}


// Transfers data from interleaved fixed point DMA buffers to a normalized, multi-channel float buffer
void deinterleave_and_convert(
        float32_t output_buf[6][SAMPLES_PER_HALF_PER_CHAN],
        const int16_t *rx_buf,
        const uint8_t i_rchannel,
        const uint8_t i_lchannel)
{
    for (uint32_t i = 0; i < SAMPLES_PER_HALF_PER_CHAN; ++i)
    {
        int16_t left_rx  = rx_buf[i * 2 + 0];
        int16_t right_rx = rx_buf[i * 2 + 1];

        output_buf[i_lchannel][i] = (float32_t) left_rx;
        output_buf[i_rchannel][i] = (float32_t) right_rx;
    }
}

// Process filled half of buffer
void process_samples(float32_t output_buf[6][SAMPLES_PER_HALF_PER_CHAN])
{
   // Do shit
}



// HAL callback functions
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if      (hsai->Instance == hsai_BlockA1.Instance) flag_sai1a_half0_filled = 1;
    else if (hsai->Instance == hsai_BlockB1.Instance) flag_sai1b_half0_filled = 1;
    else if (hsai->Instance == hsai_BlockA4.Instance) flag_sai4a_half0_filled = 1;
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if      (hsai->Instance == hsai_BlockA1.Instance) flag_sai1a_half1_filled = 1;
    else if (hsai->Instance == hsai_BlockB1.Instance) flag_sai1b_half1_filled = 1;
    else if (hsai->Instance == hsai_BlockA4.Instance) flag_sai4a_half1_filled = 1;
}





