################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/startup_ARMCM55.c \
../Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/system_ARMCM55.c 

OBJS += \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/startup_ARMCM55.o \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/system_ARMCM55.o 

C_DEPS += \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/startup_ARMCM55.d \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/system_ARMCM55.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/%.o Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/%.su Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/%.cyclo: ../Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/%.c Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -D__FPU_PRESENT=1 -DUSE_HAL_DRIVER -DSTM32H723xx -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_PWR_LDO_SUPPLY -c -I../Core/Inc -I../Drivers/CMSIS/DSP/Include -I../Drivers/CMSIS/DSP/PrivateInclude -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-cmsis_build-2f-RTE-2f-Device-2f-ARMCM55

clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-cmsis_build-2f-RTE-2f-Device-2f-ARMCM55:
	-$(RM) ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/startup_ARMCM55.cyclo ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/startup_ARMCM55.d ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/startup_ARMCM55.o ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/startup_ARMCM55.su ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/system_ARMCM55.cyclo ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/system_ARMCM55.d ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/system_ARMCM55.o ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM55/system_ARMCM55.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-cmsis_build-2f-RTE-2f-Device-2f-ARMCM55

