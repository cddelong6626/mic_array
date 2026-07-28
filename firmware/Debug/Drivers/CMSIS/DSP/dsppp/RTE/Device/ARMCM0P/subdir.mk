################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/startup_ARMCM0plus.c \
../Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/system_ARMCM0plus.c 

OBJS += \
./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/startup_ARMCM0plus.o \
./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/system_ARMCM0plus.o 

C_DEPS += \
./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/startup_ARMCM0plus.d \
./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/system_ARMCM0plus.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/%.o Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/%.su Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/%.cyclo: ../Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/%.c Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_HAL_DRIVER -DSTM32H723xx -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_PWR_LDO_SUPPLY -c -I../Core/Inc -I../Drivers/CMSIS/DSP/Include -I../Drivers/CMSIS/DSP/PrivateInclude -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DSP-2f-dsppp-2f-RTE-2f-Device-2f-ARMCM0P

clean-Drivers-2f-CMSIS-2f-DSP-2f-dsppp-2f-RTE-2f-Device-2f-ARMCM0P:
	-$(RM) ./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/startup_ARMCM0plus.cyclo ./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/startup_ARMCM0plus.d ./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/startup_ARMCM0plus.o ./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/startup_ARMCM0plus.su ./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/system_ARMCM0plus.cyclo ./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/system_ARMCM0plus.d ./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/system_ARMCM0plus.o ./Drivers/CMSIS/DSP/dsppp/RTE/Device/ARMCM0P/system_ARMCM0plus.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DSP-2f-dsppp-2f-RTE-2f-Device-2f-ARMCM0P

