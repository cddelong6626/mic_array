################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/dsppp/tests/bench.c 

OBJS += \
./Drivers/CMSIS/DSP/dsppp/tests/bench.o 

C_DEPS += \
./Drivers/CMSIS/DSP/dsppp/tests/bench.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/dsppp/tests/%.o Drivers/CMSIS/DSP/dsppp/tests/%.su Drivers/CMSIS/DSP/dsppp/tests/%.cyclo: ../Drivers/CMSIS/DSP/dsppp/tests/%.c Drivers/CMSIS/DSP/dsppp/tests/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_HAL_DRIVER -DSTM32H723xx -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_PWR_LDO_SUPPLY -c -I../Core/Inc -I../Drivers/CMSIS/DSP/Include -I../Drivers/CMSIS/DSP/PrivateInclude -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DSP-2f-dsppp-2f-tests

clean-Drivers-2f-CMSIS-2f-DSP-2f-dsppp-2f-tests:
	-$(RM) ./Drivers/CMSIS/DSP/dsppp/tests/bench.cyclo ./Drivers/CMSIS/DSP/dsppp/tests/bench.d ./Drivers/CMSIS/DSP/dsppp/tests/bench.o ./Drivers/CMSIS/DSP/dsppp/tests/bench.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DSP-2f-dsppp-2f-tests

