################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float16.neonintrisic.c \
../Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float32.neonintrisic.c \
../Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.c \
../Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.neonintrisic.c \
../Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_init.c \
../Drivers/CMSIS/DSP/Ne10/NE10_fft_float16.neonintrinsic.c \
../Drivers/CMSIS/DSP/Ne10/NE10_fft_float32.neonintrinsic.c \
../Drivers/CMSIS/DSP/Ne10/NE10_fft_int16.neonintrinsic.c \
../Drivers/CMSIS/DSP/Ne10/NE10_fft_int32.neonintrinsic.c \
../Drivers/CMSIS/DSP/Ne10/NE10_rfft_float16.neonintrinsic.c \
../Drivers/CMSIS/DSP/Ne10/NE10_rfft_float32.neonintrinsic.c 

OBJS += \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float16.neonintrisic.o \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float32.neonintrisic.o \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.o \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.neonintrisic.o \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_init.o \
./Drivers/CMSIS/DSP/Ne10/NE10_fft_float16.neonintrinsic.o \
./Drivers/CMSIS/DSP/Ne10/NE10_fft_float32.neonintrinsic.o \
./Drivers/CMSIS/DSP/Ne10/NE10_fft_int16.neonintrinsic.o \
./Drivers/CMSIS/DSP/Ne10/NE10_fft_int32.neonintrinsic.o \
./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float16.neonintrinsic.o \
./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float32.neonintrinsic.o 

C_DEPS += \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float16.neonintrisic.d \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float32.neonintrisic.d \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.d \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.neonintrisic.d \
./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_init.d \
./Drivers/CMSIS/DSP/Ne10/NE10_fft_float16.neonintrinsic.d \
./Drivers/CMSIS/DSP/Ne10/NE10_fft_float32.neonintrinsic.d \
./Drivers/CMSIS/DSP/Ne10/NE10_fft_int16.neonintrinsic.d \
./Drivers/CMSIS/DSP/Ne10/NE10_fft_int32.neonintrinsic.d \
./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float16.neonintrinsic.d \
./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float32.neonintrinsic.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/Ne10/%.o Drivers/CMSIS/DSP/Ne10/%.su Drivers/CMSIS/DSP/Ne10/%.cyclo: ../Drivers/CMSIS/DSP/Ne10/%.c Drivers/CMSIS/DSP/Ne10/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_HAL_DRIVER -DSTM32H723xx -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_PWR_LDO_SUPPLY -c -I../Core/Inc -I../Drivers/CMSIS/DSP/Include -I../Drivers/CMSIS/DSP/PrivateInclude -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DSP-2f-Ne10

clean-Drivers-2f-CMSIS-2f-DSP-2f-Ne10:
	-$(RM) ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float16.neonintrisic.cyclo ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float16.neonintrisic.d ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float16.neonintrisic.o ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float16.neonintrisic.su ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float32.neonintrisic.cyclo ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float32.neonintrisic.d ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float32.neonintrisic.o ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_float32.neonintrisic.su ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.cyclo ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.d ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.neonintrisic.cyclo ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.neonintrisic.d ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.neonintrisic.o ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.neonintrisic.su ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.o ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_generic_int32.su ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_init.cyclo ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_init.d ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_init.o ./Drivers/CMSIS/DSP/Ne10/CMSIS_NE10_fft_init.su ./Drivers/CMSIS/DSP/Ne10/NE10_fft_float16.neonintrinsic.cyclo ./Drivers/CMSIS/DSP/Ne10/NE10_fft_float16.neonintrinsic.d ./Drivers/CMSIS/DSP/Ne10/NE10_fft_float16.neonintrinsic.o ./Drivers/CMSIS/DSP/Ne10/NE10_fft_float16.neonintrinsic.su ./Drivers/CMSIS/DSP/Ne10/NE10_fft_float32.neonintrinsic.cyclo ./Drivers/CMSIS/DSP/Ne10/NE10_fft_float32.neonintrinsic.d ./Drivers/CMSIS/DSP/Ne10/NE10_fft_float32.neonintrinsic.o ./Drivers/CMSIS/DSP/Ne10/NE10_fft_float32.neonintrinsic.su ./Drivers/CMSIS/DSP/Ne10/NE10_fft_int16.neonintrinsic.cyclo ./Drivers/CMSIS/DSP/Ne10/NE10_fft_int16.neonintrinsic.d ./Drivers/CMSIS/DSP/Ne10/NE10_fft_int16.neonintrinsic.o ./Drivers/CMSIS/DSP/Ne10/NE10_fft_int16.neonintrinsic.su ./Drivers/CMSIS/DSP/Ne10/NE10_fft_int32.neonintrinsic.cyclo ./Drivers/CMSIS/DSP/Ne10/NE10_fft_int32.neonintrinsic.d ./Drivers/CMSIS/DSP/Ne10/NE10_fft_int32.neonintrinsic.o ./Drivers/CMSIS/DSP/Ne10/NE10_fft_int32.neonintrinsic.su ./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float16.neonintrinsic.cyclo ./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float16.neonintrinsic.d ./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float16.neonintrinsic.o ./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float16.neonintrinsic.su ./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float32.neonintrinsic.cyclo ./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float32.neonintrinsic.d ./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float32.neonintrinsic.o ./Drivers/CMSIS/DSP/Ne10/NE10_rfft_float32.neonintrinsic.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DSP-2f-Ne10

