################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.c \
../Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/system_ARMCM35P.c 

S_UPPER_SRCS += \
../Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.S 

OBJS += \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.o \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/system_ARMCM35P.o 

S_UPPER_DEPS += \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.d 

C_DEPS += \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.d \
./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/system_ARMCM35P.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/%.o: ../Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/%.S Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -DDEBUG -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"
Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/%.o Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/%.su Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/%.cyclo: ../Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/%.c Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_HAL_DRIVER -DSTM32H723xx -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_PWR_LDO_SUPPLY -c -I../Core/Inc -I../Drivers/CMSIS/DSP/Include -I../Drivers/CMSIS/DSP/PrivateInclude -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-cmsis_build-2f-RTE-2f-Device-2f-ARMCM35P_DSP_FP

clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-cmsis_build-2f-RTE-2f-Device-2f-ARMCM35P_DSP_FP:
	-$(RM) ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.cyclo ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.d ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.o ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/startup_ARMCM35P.su ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/system_ARMCM35P.cyclo ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/system_ARMCM35P.d ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/system_ARMCM35P.o ./Drivers/CMSIS/DSP/Testing/cmsis_build/RTE/Device/ARMCM35P_DSP_FP/system_ARMCM35P.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-cmsis_build-2f-RTE-2f-Device-2f-ARMCM35P_DSP_FP

