################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Common/minirtos.c 

OBJS += \
./Common/minirtos.o 

C_DEPS += \
./Common/minirtos.d 


# Each subdirectory must supply rules for building sources it contributes
Common/%.o Common/%.su Common/%.cyclo: ../Common/%.c Common/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Sourabh_Workspace/My_Personal_Project_Workspaces/MicroScheduler/MiniRTOS/Common" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Common

clean-Common:
	-$(RM) ./Common/minirtos.cyclo ./Common/minirtos.d ./Common/minirtos.o ./Common/minirtos.su

.PHONY: clean-Common

