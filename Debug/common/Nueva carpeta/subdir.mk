################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
ASM_SRCS += \
../common/Nueva\ carpeta/44binit.asm 

OBJS += \
./common/Nueva\ carpeta/44binit.o 

ASM_DEPS += \
./common/Nueva\ carpeta/44binit.d 


# Each subdirectory must supply rules for building sources it contributes
common/Nueva\ carpeta/44binit.o: ../common/Nueva\ carpeta/44binit.asm
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -I"C:\Users\guest\WORKSPACE\p2\common" -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"common/Nueva carpeta/44binit.d" -MT"common/Nueva\ carpeta/44binit.d" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


