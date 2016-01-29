################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bt_gateway.c \
../cJSON.c 

OBJS += \
./bt_gateway.o \
./cJSON.o 

C_DEPS += \
./bt_gateway.d \
./cJSON.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/home/liubo/work/yunba-c-sdk/src -include/home/liubo/work/yunba-c-sdk/src/yunba.h -include/home/liubo/work/yunba-c-sdk/src/cJSON.h -include/home/liubo/work/yunba-c-sdk/src/yunba_common.h -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


