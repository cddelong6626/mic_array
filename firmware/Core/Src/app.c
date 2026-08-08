/*
 * app.c
 *
 *  Created on: Jul 12, 2026
 *      Author: Cole Delong
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "arm_math.h"
#include "app.h"
#include "sk9822.h"


// Adjustable parameters
#define MAX_LAG_SAMPLES             18
#define FRAMES_PER_LED_UPDATE       10
#define SAMPLES_PER_HALF_PER_CHAN   1024

// Other parameters
#define SAMPLE_RATE                 53710
#define N_CHANS                     6
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
arm_biquad_casd_df1_inst_f32 iirfilt_conf[N_CHANS];
float32_t biquad_state[N_CHANS][4 * 2] = {0};
const float32_t biquad_coeffs[5 * 2] = {    // 4th order DF1 SOS bandpass, 200Hz to 4000 Hz
    // Stage 1
    0.0324118016379227, 0.0648236032758455, 0.0324118016379227, 1.52225273968609, -0.705601494804826,
    // Stage 2
    0.0536542717590883, 0.107308543518177, 0.0536542717590883, 1.25996445342187, -0.411721719482958
};

float32_t fft_buf[N_CHANS][FFT_LEN];
float32_t gccphat_buf[N_CHAN_PAIRS][FFT_LEN];
float32_t scratch_buf[FFT_LEN];
float32_t tdoa_estimate_sec[N_CHAN_PAIRS];

// LED update state variable
uint32_t n_frames_since_led_update = 0;
float32_t u_sum[2] = {0.0f};
uint8_t i_led_cur = 0;


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

    // Initialize the Biquad cascade IIR filter
    for (int i = 0; i < N_CHANS; ++i)
        arm_biquad_cascade_df1_init_f32(&iirfilt_conf[i], 2, biquad_coeffs, biquad_state[i]);

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
            n_frames_since_led_update++;
        }
        if (flag_chan01_half1_ready && flag_chan23_half1_ready && flag_chan45_half1_ready)
        {
            flag_chan01_half1_ready = 0;
            flag_chan23_half1_ready = 0;
            flag_chan45_half1_ready = 0;

            process_samples(audio_buf[1]);
            n_frames_since_led_update++;
        }

        // TODO
        if (n_frames_since_led_update >= FRAMES_PER_LED_UPDATE)
        {
            float32_t theta;
            arm_atan2_f32(u_sum[1], u_sum[0], &theta);

            int i_led_new = round((theta + PI) * N_LEDS/(2*PI));
            i_led_new %= 12;

            sk9822_edit_led(i_led_cur, 255, 255, 255, 0);
            i_led_cur = i_led_new;
            sk9822_edit_led(i_led_cur, 255, 255, 255, 31);
            sk9822_send_update();

            n_frames_since_led_update = 0;
            u_sum[0] = 0.0f;
            u_sum[1] = 0.0f;
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
        float32_t left_rx  = (float32_t)rx_buf[i * 2 + 0];
        float32_t right_rx = (float32_t)rx_buf[i * 2 + 1];

        output_buf[i_lchannel][i] = left_rx;
        output_buf[i_rchannel][i] = right_rx;

    }
}

// Process the filled half of the audio buffer
void process_samples(float32_t input_buf[N_CHANS][SAMPLES_PER_HALF_PER_CHAN])
{
    // Compute the FFT of each channel
    for (uint8_t i = 0; i < N_CHANS; ++i)
    {
        // Anti-spatial aliasing bandpass filter: 200Hz to 4000Hz passband
        arm_biquad_cascade_df1_f32(&iirfilt_conf[i], input_buf[i], scratch_buf, FFT_LEN);

        /*
         * FFT Output:
         * { real(X[0]), real(X[N/2]), real(X[1]), imag(X[1]), real(X[2]), imag(X[2]), ..., real(X[N/2-1]), imag(X[N/2-1]) }
         */
        arm_rfft_fast_f32(&rfft_conf, scratch_buf, fft_buf[i], 0);
    }

    // Iterate over upper triangle pairs
    int i_pair = 0;
    for (int i = 0; i < N_CHANS; ++i)
    {
        // Only compute upper triangle to avoid redundant computation
        for (int j = i + 1; j < N_CHANS; ++j)
        {
            // Compute the GCC-PHAT element-wise cross spectrums via multiplication and normalization of the FFT outputs
            // The first two elements are not relevant to GCC-PHAT
            gccphat_buf[i_pair][0] = 0;
            gccphat_buf[i_pair][1] = 0;

            // The rest are interleaved complex pairs. Conjugate and multiply
            arm_cmplx_conj_f32(&fft_buf[j][2], &scratch_buf[2], N_CMPX_SAMPLES);
            arm_cmplx_mult_cmplx_f32(&fft_buf[i][2], &scratch_buf[2], &gccphat_buf[i_pair][2], N_CMPX_SAMPLES);

            // Element-wise PHAT normalization
            arm_cmplx_mag_f32(&gccphat_buf[i_pair][2], scratch_buf, N_CMPX_SAMPLES);
            for (int k = 0; k < N_CMPX_SAMPLES; ++k) {
                if (scratch_buf[k] > 1e-6f)
                {
                    scratch_buf[k] = powf(scratch_buf[k], -0.6);
                }
                else
                {
                    scratch_buf[k] = 0.0f; // Safely zero out bins with no energy
                }
            }
            arm_cmplx_mult_real_f32(&gccphat_buf[i_pair][2], scratch_buf, &gccphat_buf[i_pair][2], N_CMPX_SAMPLES);

            // Compute cross-correlations between channels via iFFT of GCC-PHAT cross spectrums
            arm_rfft_fast_f32(&rfft_conf, gccphat_buf[i_pair], scratch_buf, 1);

            // Estimate time difference of arrival (TDOA)
            float32_t tdoa_estimate_samp_ij;
            float32_t max_pos, max_neg;
            uint32_t idx_pos, idx_neg;
            arm_max_f32(&scratch_buf[0], MAX_LAG_SAMPLES + 1, &max_pos, &idx_pos);
            arm_max_f32(&scratch_buf[FFT_LEN - MAX_LAG_SAMPLES], MAX_LAG_SAMPLES, &max_neg, &idx_neg);

            uint32_t idx_max = (max_pos >= max_neg)
                ? idx_pos
                : (FFT_LEN - MAX_LAG_SAMPLES + idx_neg);

            // Parabolic interpolation
            float32_t delta;
            if (idx_max > 0 && idx_max < FFT_LEN - 1)
            {
                float32_t y1 = scratch_buf[idx_max - 1];
                float32_t y2 = scratch_buf[idx_max];
                float32_t y3 = scratch_buf[idx_max + 1];
                delta = (y1 - y3) / (2.0f * (y1 - 2.0f*y2 + y3));
            }
            else
            {
                delta = 0.0f;
            }

            tdoa_estimate_samp_ij = (float32_t) idx_max + delta;

            // Account for circular iFFT output
            if (tdoa_estimate_samp_ij > (FFT_LEN/2))    // Account for circular iFFT output
            {
                tdoa_estimate_samp_ij = tdoa_estimate_samp_ij - (float32_t)FFT_LEN;
            }
            else
            {
                tdoa_estimate_samp_ij = tdoa_estimate_samp_ij;
            }

            // Convert TDOA estimate from units of samples to units of seconds
            tdoa_estimate_sec[i_pair] = tdoa_estimate_samp_ij / (float32_t)SAMPLE_RATE;
            i_pair++;

        }
    }


    // Least squares geometry solver (M generated with geometry_solver_matrix_generator.m)
    const float M[2][15] = {
        {3.585270547f, 1.027346524f, 2.314476061f, 2.420642327f, 0.931284531f, -2.557924022f, -1.270794486f, -1.164628220f, -2.653986016f, 1.287129537f, 1.393295803f, -0.096061993f, 0.106166266f, -1.383191530f, -1.489357796f},
        {-1.568963232f, -1.406235762f, -0.147240040f, -2.708135246f, 0.910928704f, 0.162727471f, 1.421723193f, -1.139172014f, 2.479891936f, 1.258995722f, -1.301899484f, 2.317164466f, -2.560895206f, 1.058168744f, 3.619063950f}
    };

    float32_t u[2] = {0.0f, 0.0f};
    for (int i = 0; i < 15; ++i)
    {
        u[0] -= M[0][i] * tdoa_estimate_sec[i];
        u[1] -= M[1][i] * tdoa_estimate_sec[i];
    }

    // Add vector components for confidence-weighted circular sum
    u_sum[0] += u[0];
    u_sum[1] += u[1];
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





