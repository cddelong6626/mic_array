# Real-Time GCC-PHAT-&beta; Microphone Array
A real-time embedded direction-of-arrival (DOA) estimation system using a circular 6-mic array, GCC-PHAT-&beta; time delay of arrival (TDOA) estimation, and a least squares geometry solver running on an STM32H7. 

## Demo

< Include video/gif >  
This system captures audio synchronously across the six microphone channels at ~48kHz, computes GCC-PHAT-&beta; cross correlations for all 15 mic pairs, and estimates the direction of arrival in real-time. 

## System Overview

## Hardware


## Algorithm

### GCC-PHAT-&beta;

### Least Square Geometry Solver

## Embedded Implementation

### Processing Pipeline/Software Architecture

### Memory/DMA architecture

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
