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

### GCC-PHAT-&beta; TDOA Estimation
This algorithm estimates the time delay of arrival (TDOA) between each pair of mics in the array to use in the geometry solver. For a 6-mic array, this amounts to $\binom{6}{2} = 15 \text{ pairs.}$ This is accomplished using FFT-accelerated cross correlations of PHAT-&beta; transformed signals.    

![Digital signal processing pipeline flowchart](images/dsp_pipeline.svg)

For each audio channel: 
1. Bandpass filter each channel with stopbands at approximately $\(100\text{ Hz}\)$ and $\(3\text{ kHz}\).$ The lower cutoff attenuates low-frequency noise, such as "room rumble" from air vents. The upper cutoff limits spatial aliasing. For a circular array with radius $\(r = 4\text{ cm}\)$, the approximate spatial aliasing frequency is $f_{\text{alias}} \approx \frac{343 \text{ m/s}}{2(0.04 \text{ m})} \approx 4.3\text{ kHz}.$ This filter is implemented as a 4th order IIR SOS filter for high performance and good numerical stability. The CMSIS-DSP coefficients for this are generated using the MATLAB script "generate_filter_coefs.m". 
2. FFT each channel.

For each pair of audio channels $i$, $j$: 
<ol start="3">
  <li>
    Elementwise multiply channel $i$ with the complex conjugate of channel $j$. This is equivalent to cross-correlation in the time domain. 
  </li>
  <li>
    PHAT-&beta; transform the signal by dividing the product of the previous step by its own magnitude to the power of &beta;. When $\beta = 1$, this is equivalent to a PHAT transform, which completely strips the signal of its phase information. When $\beta = 0$, all denominators are 0 and the signal is uneffected, making this algorithm equivalent to GCC. GCC works well with narrowband sources and worse with wideband sources and multipath reflections. GCC-PHAT works well with wideband sources and in reverberant environments but very poorly with narrowband sources. A &beta; between 0 and 1 allows for a blend between these tradeoffs, retaining some amount of magnitude information in the signal. A &beta; between 0.5 and 0.7 is recommended. 
  </li>
  <li>
    Inverse FFT the signal to convert it back to the time domain. 
  </li>
  <li>
    Find the peak of the time domain signal within the bounds defined by the maximum TDOA given the microphone array geometry. It's best to use sub-sample interpolation as the TDOA resolution is otherwise low. The result of this is the TDOA estimate between channels $i$ and $j$. 
  </li>
</ol>
 

### Least Squares Geometry Solver

Once the TDOA estimate is computed for all pairs of microphones, a least squares geometry solver is used to estimate DOA. 

<p align="center">
<img src="images/mic_array_diagram.svg" style="display: block; margin: 0 auto; width: 50%; text-align: center;" />
</p>

Trigonometry yields the following formula for the distance $d_{ij}$ and time $\tau_{ij}$ a sound wave has to travel from mic $i$ to mic $j$: 

$$ d_{ij} = \lVert \vec{r}_j - \vec{r}_i \rVert \text{ } \cos(\theta) = (\vec{r}_j - \vec{r}_i)^T \hat{u} = c \tau_{ij} $$
$$ \text{where  } c = 343 \text{ }m\text{/}s \text{, } \vec{v} = c \hat{u} $$

We now solve this equation simultaneously for all mics. Construct matrix $R$ containing all spatial offsets between microphones and vector $\vec{\tau}$ containing all estimated temporal offsets (TDOAs).

$$
R =
\begin{bmatrix}
(\vec{r}_2 - \vec{r}_1)^T \\
(\vec{r}_3 - \vec{r}_1)^T \\
\vdots \\
(\vec{r}_6 - \vec{r}_5)^T
\end{bmatrix}
,\qquad
\vec{\tau} =
\begin{bmatrix}
\tau_{12} \\
\tau_{13} \\
\vdots \\
\tau_{56}
\end{bmatrix}
$$

Combined equation:

$$ R \hat{u} = c \vec{\tau} $$

This equation has 15 equations in 2 unknowns, making it highly over constrained. Solve for least squares solution:

$$ R^T R \hat{u} = c R^T \vec{\tau} $$
$$ \hat{u} = c \( R^T R \)^{-1} R^T \vec{\tau} $$

The only part of the right hand side of this equation that is variable is $\tau$, allowing for pre-computation of $M = ( R^T R )^{-1} R^T$ (factor $c$ not relevant to DOA angle). This matrix is generated using the MATLAB script "geometry_solver_matrix_generator.m".   

The result of this algorithm is taken as an angle:  

$$\theta = \text{atan2}(\hat{u}_y, \hat{u}_x)$$

## Embedded Implementation

### Processing Pipeline/Software Architecture


### Memory/DMA architecture
<p align="center">
  <img src="images/embedded_memory_layout.svg" />
</p>

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
