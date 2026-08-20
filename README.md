# Real-Time GCC-PHAT-&beta; Microphone Array
A real-time embedded direction-of-arrival (DOA) estimation system using a circular 6-mic array, GCC-PHAT-&beta; time delay of arrival (TDOA) estimation, and a least squares geometry solver running on an STM32H7. 

## Demo

< Include video/gif >  
This system captures audio synchronously across the six microphone channels at ~48kHz, computes GCC-PHAT-&beta; cross correlations for all 15 mic pairs, and estimates the direction of arrival in real-time. 

## System Overview
![System overview flowchart](images/system_overview.svg)

## Hardware

This system samples audio synchronously and indicates DOA with a circular 6-microphone array and processes the audio data on an STM32H7 development board. 

### Microphone Array

The microphone array is a [Sipeed 6+1 Microphone Array](https://www.dfrobot.com/product-1976.html), which includes 6 MEMS mics ([MSM261S4030H0](https://dfimg.dfrobot.com/wiki/19409/SEN0526_i2s-mems-microphone_datasheet_v1.0.pdf)) evenly spaced in a circle with a 4 cm radius and 1 mic in the center that was not used. These mics communicate using I2S, with pairs of mics (0 and 1, 2 and 3, 4 and 5, and 7 by itself) sharing interfaces through TDD. The mics are sampled synchronously via precise routing as well as shared bit clock and R/L clock.

This board additionally has 12 RGB LEDs ([SK9822](https://www.normandled.com/upload/201909/SK9822%20LED%20Datasheet.pdf)) arranged in a hexagon on the opposite side of the PCB from the mics. This LED array communicates using SPI and is used for the real-time DOA indication.

The microphone array was primarily chosen for the following reasons:

* Low cost: At $11.90 before shipping, it was significantly cheaper than most microphone arrays that I could find, which were typically >$60.
* Raw microphone outputs: Most of the other microphone arrays I found included their own on-board audio processing through a dedicated IC. Since the purpose of this project was to develop the audio processing algorithm, using a mic array that already processed the audio and only using its raw microphone outputs seemed to make for a less elegant system.
* Six microphones: Most of the other arrays I found had only 2 or 4 mics. Six mics was more attractive because it makes for more of a challenge to synchronize and process the audio samples.
* Minimal features: The array's minimal features were an added bonus, since the project was primarily focused on implementing the audio processing algorithm rather than using an existing audio-processing system.

### STM32H7 Development Board

The [NUCLEO-H723ZG](https://www.st.com/en/evaluation-tools/nucleo-h723zg.html) was selected for this project for the following reasons:

* Adequate compute power: A lower-performance core like a Cortex-M4 would likely have trouble running this algorithm and likely wouldn't have the speed to add more features later on.
* SAI support: SAI support made this project significantly easier than if there had only been I2S interfaces available, as the SAI interfaces allowed me to internally synchronize all 3 I2S interfaces without any external wiring.
* Single core: A single core was preferable due to its lower complexity compared to a dual-core system.
* Internal RAM: A fair amount of internal RAM was useful because it is lower in complexity compared to external RAM, and this DSP algorithm requires a large number of buffers to handle all 6 channels and 15 pairs of channels.
* Older board: The age of this board meant there was more information about it available to use when debugging.
* Low cost: The board was relatively inexpensive compared to its competitors, though price differences were marginal. 

## DSP Pipeline

### GCC-PHAT-&beta;
![Digital signal processing pipeline flowchart](images/dsp_pipeline.svg)


### Least Square Geometry Solver

## Embedded Implementation

### Processing Pipeline/Software Architecture


### Memory/DMA architecture
![Embedded memory/DMA layout flowchart](images/embedded_memory_layout.svg)


## Performance
Sample rate
Block size
Channels
Pairwaise correlations
Latency
Block duration
CPU utilization
RAM usage
FLASH usage

## Validation
Talk about the 6 sample shift tests
Talk about extracting STM32 memory, importing into MATLAB, and testing the algorithm on it

## Results
If I can figure out how to have a speaker at a very specific angle I can do true angle vs estimated angle to find error in degrees. Might not though lowkey

## Future Work
- Better fractional interpolation if necessary
- Test zeroing out unnecessary parts of spectrum with framing
- Try higher clock speed
- Dynamic &beta;
- Add real-time beamforming (need to figure out good way to get signal off of board onto computer or speaker)
- Mechanical enclosure
- Kalman/particle filter result rather than simple circular mean

## Repository Structure

## References
