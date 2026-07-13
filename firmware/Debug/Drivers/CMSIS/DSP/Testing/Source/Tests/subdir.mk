################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata.c \
../Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata_f16.c 

OBJS += \
./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata.o \
./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata_f16.o 

C_DEPS += \
./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata.d \
./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata_f16.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/Testing/Source/Tests/%.o Drivers/CMSIS/DSP/Testing/Source/Tests/%.su Drivers/CMSIS/DSP/Testing/Source/Tests/%.cyclo: ../Drivers/CMSIS/DSP/Testing/Source/Tests/%.c Drivers/CMSIS/DSP/Testing/Source/Tests/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -D__FPU_PRESENT=1 -DUSE_HAL_DRIVER -DSTM32H723xx -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_PWR_LDO_SUPPLY -c -I../Core/Inc -I../Drivers/CMSIS/DSP/Include -I../Drivers/CMSIS/DSP/PrivateInclude -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-Source-2f-Tests

clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-Source-2f-Tests:
	-$(RM) ./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata.cyclo ./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata.d ./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata.o ./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata.su ./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata_f16.cyclo ./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata_f16.d ./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata_f16.o ./Drivers/CMSIS/DSP/Testing/Source/Tests/mfccdata_f16.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DSP-2f-Testing-2f-Source-2f-Tests

