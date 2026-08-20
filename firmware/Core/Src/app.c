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
#define CONFIDENCE_THRESHOLD        0.3     // 0 to 1
#define BETA                        0.6     // 0 to 1, recommended 0.5 to 0.7
#define N_FRAMES_PER_LED_UPDATE     5       // Higher = more stable estimate but higher latency
#define SAMPLES_PER_HALF_PER_CHAN   1024
#define MAX_LAG_SAMPLES             18      // TODO: Write constraints

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

// 6 channels: sai1_BlockA(1,2)  sai1_BlockB(3,2)  sai4_BlockA(5,4)
float32_t audio_buf[2][N_CHANS][SAMPLES_PER_HALF_PER_CHAN];

// Flags to indicate when a DMA buffer half has been filled
volatile uint8_t flag_sai1a_half0_filled = 0;
volatile uint8_t flag_sai1b_half0_filled = 0;
volatile uint8_t flag_sai4a_half0_filled = 0;
volatile uint8_t flag_sai1a_half1_filled = 0;
volatile uint8_t flag_sai1b_half1_filled = 0;
volatile uint8_t flag_sai4a_half1_filled = 0;

// Flags to indicate when an audio buffer half is ready to be processed
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
const float32_t biquad_coeffs[5 * 2] = {    // 4th order DF1 SOS bandpass, 100Hz to 3000 Hz, fs=53710 Hz
    // Stage 1
    0.0230695593792243, 0.0461391187584486, 0.0230695593792243, 1.53591407298219, -0.629300994443601,
    // Stage 2
    1, -2, 1, 1.98352395873059, -0.98366857309285
};

float32_t fft_buf[N_CHANS][FFT_LEN];
float32_t gccphat_buf[N_CHAN_PAIRS][FFT_LEN];
float32_t scratch_buf[FFT_LEN];
float32_t tdoa_estimate_s[N_CHAN_PAIRS];

// Least squares geometry solver (M generated with geometry_solver_matrix_generator.m)
const float M[2][15] = {
    {0.694444444f, 2.083333333f, 2.777777778f, 2.083333333f, 0.694444444f, 1.388888889f, 2.083333333f, 1.388888889f, 0.000000000f, 0.694444444f, 0.000000000f, -1.388888889f, -0.694444444f, -2.083333333f, -1.388888889f},
    {-1.202813061f, -1.202813061f, -0.000000000f, 1.202813061f, 1.202813061f, -0.000000000f, 1.202813061f, 2.405626122f, 2.405626122f, 1.202813061f, 2.405626122f, 2.405626122f, 1.202813061f, 1.202813061f, 0.000000000f}
};
const float32_t SPEED_OF_SOUND_MPS = 343.0f;

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

    // Initialize the Direct Form I Biquad cascade IIR filter
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
            deinterleave_and_convert(audio_buf[0], &sai1a_rx_buf[0], 1, 0);
            flag_chan01_half0_ready = 1;
        }

        if (flag_sai1b_half0_filled)
        {
            flag_sai1b_half0_filled = 0;
            deinterleave_and_convert(audio_buf[0], &sai1b_rx_buf[0], 3, 2);
            flag_chan23_half0_ready = 1;
        }

        if (flag_sai4a_half0_filled)
        {
            flag_sai4a_half0_filled = 0;
            deinterleave_and_convert(audio_buf[0], &sai4a_rx_buf[0], 5, 4);
            flag_chan45_half0_ready = 1;
        }

        if (flag_sai1a_half1_filled)
        {
            flag_sai1a_half1_filled = 0;
            deinterleave_and_convert(audio_buf[1], &sai1a_rx_buf[SAMPLES_PER_HALF], 1, 0);
            flag_chan01_half1_ready = 1;
        }

        if (flag_sai1b_half1_filled)
        {
            flag_sai1b_half1_filled = 0;
            deinterleave_and_convert(audio_buf[1], &sai1b_rx_buf[SAMPLES_PER_HALF], 3, 2);
            flag_chan23_half1_ready = 1;
        }

        if (flag_sai4a_half1_filled)
        {
            flag_sai4a_half1_filled = 0;
            deinterleave_and_convert(audio_buf[1], &sai4a_rx_buf[SAMPLES_PER_HALF], 5, 4);
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

        // TODO
        if (n_frames_since_led_update >= N_FRAMES_PER_LED_UPDATE)
        {
            float32_t confidence;
            arm_cmplx_mag_f32(u_sum, &confidence, 1);
            confidence /= N_FRAMES_PER_LED_UPDATE;

            if (confidence > CONFIDENCE_THRESHOLD)
            {
                float32_t theta_source;
                float32_t theta_led;
                float32_t dtheta;
                int8_t brightness;

                arm_atan2_f32(u_sum[1], u_sum[0], &theta_source);
                if (theta_source < 0) theta_source += 2*PI;
                for (int i = 0; i < N_LEDS; ++i)
                {
                    theta_led = i * 2*PI/N_LEDS;
                    dtheta = fabsf(theta_led - theta_source);

                    if (dtheta > PI) dtheta = 2*PI - dtheta;
                    brightness = 31 - roundf(dtheta * 134/PI);
                    if (brightness < 0) brightness = 0;
                    sk9822_edit_led(i, 255, 255, 255, brightness);
                }

                sk9822_send_update();
            }


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
        // Anti-spatial aliasing bandpass filter
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
            // The first two elements are not important for GCC-PHAT-Beta
            gccphat_buf[i_pair][0] = 0;
            gccphat_buf[i_pair][1] = 0;

            // The rest are interleaved complex pairs. Conjugate and multiply
            arm_cmplx_conj_f32(&fft_buf[j][2], &scratch_buf[2], N_CMPX_SAMPLES);
            arm_cmplx_mult_cmplx_f32(&fft_buf[i][2], &scratch_buf[2], &gccphat_buf[i_pair][2], N_CMPX_SAMPLES);

            // Element-wise PHAT-Beta normalization
            arm_cmplx_mag_f32(&gccphat_buf[i_pair][2], scratch_buf, N_CMPX_SAMPLES);
            for (int k = 0; k < N_CMPX_SAMPLES; ++k)
            {
                if (scratch_buf[k] > 1e-6f)
                {
                    scratch_buf[k] = powf(scratch_buf[k], -BETA);
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
            // Only look within +/- MAX_LAG_SAMPLES for max value. First of all, it isn't physically possible for
            // the lag to be greater than some number of samples based on the array geometry and the sample rate.
            // Second, since this code uses PHAT-Beta rather than pure PHAT, there can be peaks outside of this
            // range that don't correspond to the desired lag.
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
            tdoa_estimate_samp_ij = (float32_t)idx_max + delta;

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
            tdoa_estimate_s[i_pair] = tdoa_estimate_samp_ij / (float32_t)SAMPLE_RATE;
            i_pair++;
        }
    }

    // Least Squares Geometry Solver
    float32_t u[2] = {0.0f, 0.0f};
    for (int i = 0; i < 15; ++i)
    {
        u[0] -= M[0][i] * tdoa_estimate_s[i] * SPEED_OF_SOUND_MPS;
        u[1] -= M[1][i] * tdoa_estimate_s[i] * SPEED_OF_SOUND_MPS;
    }

    // Sum vector components over multiple frames for confidence-weighted circular sum
    u_sum[0] += u[0];
    u_sum[1] += u[1];

    n_frames_since_led_update++;
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





