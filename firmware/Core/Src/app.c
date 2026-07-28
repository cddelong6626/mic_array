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
#define N_CHANS                     6
#define SAMPLES_PER_HALF_PER_CHAN   256
#define SAMPLES_PER_HALF            (SAMPLES_PER_HALF_PER_CHAN * 2)
#define BUF_LEN                     (SAMPLES_PER_HALF * 2)
#define FFT_LEN                     SAMPLES_PER_HALF_PER_CHAN
#define N_CHAN_PAIRS                N_CHANS*(N_CHANS-1)/2
#define N_CMPX_SAMPLES              (FFT_LEN)/2 - 1

// 16-bits rather than 24-bits per sample since mic SNR only gives 10-11 bits anyway
int16_t sai1a_rx_buf[BUF_LEN] __attribute__((section(".dma_buffer")));      // DMA buffer:  cache disabled, D2 ram
int16_t sai1b_rx_buf[BUF_LEN] __attribute__((section(".dma_buffer")));
int16_t sai4a_rx_buf[BUF_LEN] __attribute__((section(".bdma_buffer")));     // BDMA buffer: cache disabled, D3 ram

// 6 channels: sai1_BlockA(0,1)  sai1_BlockB(2,3)  sai4_BlockA(4,5)
float32_t audio_buf[2][N_CHANS][SAMPLES_PER_HALF_PER_CHAN];

// Flags to indicate when a DMA buffer half has been filled
volatile uint8_t flag_sai1a_half0_filled = 0;
volatile uint8_t flag_sai1b_half0_filled = 0;
volatile uint8_t flag_sai4a_half0_filled = 0;
volatile uint8_t flag_sai1a_half1_filled = 0;
volatile uint8_t flag_sai1b_half1_filled = 0;
volatile uint8_t flag_sai4a_half1_filled = 0;

// Flags to indicate when a buffer half is ready to be processed
uint8_t flag_chan01_half0_ready = 0;
uint8_t flag_chan23_half0_ready = 0;
uint8_t flag_chan45_half0_ready = 0;
uint8_t flag_chan01_half1_ready = 0;
uint8_t flag_chan23_half1_ready = 0;
uint8_t flag_chan45_half1_ready = 0;

// GCC-PHAT buffers and configuration
arm_rfft_fast_instance_f32 rfft_conf;
float32_t fft_buf[N_CHANS][FFT_LEN];
float32_t gccphat_buf[N_CHANS-1][N_CHANS][FFT_LEN];
float32_t temp_buf[FFT_LEN];
int32_t tdoa_estimate[N_CHANS-1][N_CHANS];


// Function prototypes
void deinterleave_and_convert(
        float32_t output_buf[N_CHANS][SAMPLES_PER_HALF_PER_CHAN],
        const int16_t *rx_buf,
        const uint8_t i_rchannel,
        const uint8_t i_lchannel);
void process_samples(float32_t output_buf[N_CHANS][SAMPLES_PER_HALF_PER_CHAN]);


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

    // Initialize the RFFT instance
    arm_rfft_fast_init_f32(&rfft_conf, FFT_LEN);

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
            flag_chan45_half1_ready = 1;
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
        float32_t output_buf[N_CHANS][SAMPLES_PER_HALF_PER_CHAN],
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

// Process the filled half of the audio buffer
void process_samples(float32_t input_buf[N_CHANS][SAMPLES_PER_HALF_PER_CHAN])
{
    // Equations found here: https://xavieranguera.com/phdthesis/node92.html

    // Compute the FFT of each channel
    for (uint8_t i = 0; i < N_CHANS; ++i)
    {
        /*
         * Output:
         * { real(X[0]), real(X[N/2]), real(X[1]), imag(X[1]), real(X[2]), imag(X[2]), ..., real(X[N/2-1]), imag(X[N/2-1]) }
         */
        arm_rfft_fast_f32(&rfft_conf, input_buf[i], fft_buf[i], 0);
    }

    // Iterate over upper triangle pairs
    for (int i = 0; i < N_CHANS; ++i)
    {
        for (int j = 0; j < N_CHANS; ++j)
        {
            // Only compute upper triangle to avoid redundant computation
            if (i >= j) continue;

            // Compute the GCC-PHAT element-wise cross spectrums via multiplication and normalization of the FFT outputs
            // The first two elements are not relevant to GCC-PHAT
            gccphat_buf[i][j][0] = 0;   // DC component only contributes offset
            gccphat_buf[i][j][1] = 0;   // Nyquist component only contributes phase

            // The rest are interleaved complex pairs. Conjugate and multiply
            arm_cmplx_conj_f32(&fft_buf[j][2], &temp_buf[2], N_CMPX_SAMPLES);
            arm_cmplx_mult_cmplx_f32(&fft_buf[i][2], &temp_buf[2], &gccphat_buf[i][j][2], N_CMPX_SAMPLES);

            // Element-wise PHAT normalization
            arm_cmplx_mag_f32(&gccphat_buf[i][j][2], temp_buf, N_CMPX_SAMPLES);
            for (int k = 0; k < N_CMPX_SAMPLES; ++k) {
                if (temp_buf[k] > 1e-6f) {
                    temp_buf[k] = 1.0f / temp_buf[k];
                } else {
                    temp_buf[k] = 0.0f; // Safely zero out bins with no energy
                }
            }
            arm_cmplx_mult_real_f32(&gccphat_buf[i][j][2], temp_buf, &gccphat_buf[i][j][2], N_CMPX_SAMPLES);

            // Compute cross-correlations between channels via iFFT of GCC-PHAT cross spectrums
            arm_rfft_fast_f32(&rfft_conf, gccphat_buf[i][j], temp_buf, 1);

            // Estimate time difference of arrival (TDOA)
            uint32_t max_index;
            arm_max_f32(temp_buf, FFT_LEN, NULL, &max_index);

            if (max_index > (FFT_LEN/2))    // Account for circular iFFT output
            {
                tdoa_estimate[i][j] = (int32_t) max_index - (int32_t) FFT_LEN;
            }
            else
            {
                tdoa_estimate[i][j] = max_index;
            }


        }
    }



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





