################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../8led.c \
../Bmp.c \
../button.c \
../lcd.c \
../led.c \
../main.c \
../sudoku_2015.c \
../timer.c 

OBJS += \
./8led.o \
./Bmp.o \
./button.o \
./lcd.o \
./led.o \
./main.o \
./sudoku_2015.o \
./timer.o 

C_DEPS += \
./8led.d \
./Bmp.d \
./button.d \
./lcd.d \
./led.d \
./main.d \
./sudoku_2015.d \
./timer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Users\guest\WORKSPACE\p2\common" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -std=c99 -lm -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


