/*
 * app.c  —  DIAGNOSTIC BUILD: per-channel power meter
 *
 *  Purpose: print a rolling-average power level for each of the 6 mic
 *  channels so you can move a speaker around and match "loudest channel"
 *  to "physical mic position", confirming (or fixing) your channel map:
 *      sai1_BlockA -> chan 0,1
 *      sai1_BlockB -> chan 2,3
 *      sai4_BlockA -> chan 4,5
 *
 *  This strips out the GCC-PHAT / TDOA / geometry pipeline entirely —
 *  swap back to your original app.c once you've confirmed the wiring.
 *
 *  Created on: Jul 12, 2026 (diagnostic variant Aug 2026)
 *      Author: Cole Delong
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "arm_math.h"
#include "app.h"
#include "sk9822.h"

// Double buffer made using circular buffer
#define SAMPLE_RATE                 53710
#define N_CHANS                     6
#define SAMPLES_PER_HALF_PER_CHAN   1024
#define SAMPLES_PER_HALF            (SAMPLES_PER_HALF_PER_CHAN * 2)
#define BUF_LEN                     (SAMPLES_PER_HALF * 2)
#define FFT_LEN                     SAMPLES_PER_HALF_PER_CHAN

// Power-meter tuning
#define FRAMES_PER_PRINT            26      // ~0.5s at 53710Hz / 1024-sample halves
#define POWER_EMA_ALPHA             0.05f   // rolling-average smoothing factor
#define METER_DB_FLOOR              -60.0f  // bar-graph bottom of scale
#define METER_DB_CEIL               0.0f    // bar-graph top of scale
#define METER_BAR_WIDTH             40

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

// Flags to indicate when a buffer half is ready to be processed
uint8_t flag_chan01_half0_ready = 0;
uint8_t flag_chan23_half0_ready = 0;
uint8_t flag_chan45_half0_ready = 0;
uint8_t flag_chan01_half1_ready = 0;
uint8_t flag_chan23_half1_ready = 0;
uint8_t flag_chan45_half1_ready = 0;

// Bandpass filter (kept from main pipeline) - removes DC offset and limits
// the power reading to the same passband GCC-PHAT operates on, so what you
// see here is representative of what the TDOA stage actually "hears".
arm_biquad_casd_df1_inst_f32 iirfilt_conf[N_CHANS];
float32_t biquad_state[N_CHANS][4 * 2] = {0};
const float32_t biquad_coeffs[5 * 2] = {    // 4th order DF1 SOS bandpass, 200Hz to 4000 Hz
    // Stage 1
        0.0324118016379227, 0.0648236032758455, 0.0324118016379227, 1.52225273968609, -0.705601494804826,
    // Stage 2
        0.0536542717590883, 0.107308543518177, 0.0536542717590883, 1.25996445342187, -0.411721719482958
};
float32_t scratch_buf[FFT_LEN];

// Rolling-average power state
float32_t power_avg[N_CHANS] = {0};
uint16_t n_frames_since_print = 0;

// Function prototypes
void deinterleave_and_convert(
        float32_t output_buf[N_CHANS][SAMPLES_PER_HALF_PER_CHAN],
        const int16_t *rx_buf,
        const uint8_t i_rchannel,
        const uint8_t i_lchannel);
void process_samples(float32_t output_buf[N_CHANS][SAMPLES_PER_HALF_PER_CHAN]);
void print_power_meter(void);


// Initialization
HAL_StatusTypeDef app_init(void)
{
    HAL_StatusTypeDef ret_status = HAL_OK;

    // Initialize LED array
    ret_status |= sk9822_init(&hspi4);

    // Turn all LEDs off - not needed for the power meter and bright at close range
    for (int i = 0; i < N_LEDS; ++i)
        sk9822_edit_led(i, 0, 0, 0, 0);
    sk9822_send_update();

    // Slave block(s) must be armed before the master starts clocking
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)sai1b_rx_buf, BUF_LEN);
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockA4, (uint8_t *)sai4a_rx_buf, BUF_LEN);
    ret_status |= HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)sai1a_rx_buf, BUF_LEN);

    // Initialize the Biquad cascade IIR filter
    for (int i = 0; i < N_CHANS; ++i)
        arm_biquad_cascade_df1_init_f32(&iirfilt_conf[i], 2, biquad_coeffs, biquad_state[i]);

    printf("\r\n--- Mic array power meter (channels 0-5) ---\r\n");
    printf("Move a speaker near each mic and watch which channel jumps.\r\n\r\n");

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

        if (n_frames_since_print >= FRAMES_PER_PRINT)
        {
            print_power_meter();
            n_frames_since_print = 0;
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

// Process the filled half of the audio buffer: bandpass filter, compute
// mean-square power, and fold it into a rolling (EMA) average per channel.
void process_samples(float32_t input_buf[N_CHANS][SAMPLES_PER_HALF_PER_CHAN])
{
    for (uint8_t ch = 0; ch < N_CHANS; ++ch)
    {
        // Anti-spatial aliasing bandpass filter: 200Hz to 4000Hz passband
        arm_biquad_cascade_df1_f32(&iirfilt_conf[ch], input_buf[ch], scratch_buf, FFT_LEN);

        // Mean-square power of this half-buffer for this channel
        float32_t sum_sq = 0.0f;
        arm_power_f32(scratch_buf, FFT_LEN, &sum_sq);
        float32_t power_inst = sum_sq / (float32_t)FFT_LEN;

        // Exponential moving average -> rolling average power
        power_avg[ch] = (1.0f - POWER_EMA_ALPHA) * power_avg[ch] + POWER_EMA_ALPHA * power_inst;
    }

    n_frames_since_print++;
}

// Print a simple table + bar graph of rolling-average power per channel, in dB.
void print_power_meter(void)
{
    char bar[METER_BAR_WIDTH + 1];

    printf("\r\n");
    for (uint8_t ch = 0; ch < N_CHANS; ++ch)
    {
        float32_t db = 10.0f * log10f(power_avg[ch] + 1e-6f);

        float32_t frac = (db - METER_DB_FLOOR) / (METER_DB_CEIL - METER_DB_FLOOR);
        if (frac < 0.0f) frac = 0.0f;
        if (frac > 1.0f) frac = 1.0f;
        int n_filled = (int)(frac * METER_BAR_WIDTH + 0.5f);

        for (int k = 0; k < METER_BAR_WIDTH; ++k)
            bar[k] = (k < n_filled) ? '#' : '-';
        bar[METER_BAR_WIDTH] = '\0';

        printf("ch%d [%s] %6.1f dB\r\n", ch, bar, db);
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
