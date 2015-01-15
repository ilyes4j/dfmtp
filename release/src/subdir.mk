################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cli.c \
../src/main.c \
../src/minimal_traversal_processor.c \
../src/transaction_loader.c 

OBJS += \
./src/cli.o \
./src/main.o \
./src/minimal_traversal_processor.o \
./src/transaction_loader.o 

C_DEPS += \
./src/cli.d \
./src/main.d \
./src/minimal_traversal_processor.d \
./src/transaction_loader.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


