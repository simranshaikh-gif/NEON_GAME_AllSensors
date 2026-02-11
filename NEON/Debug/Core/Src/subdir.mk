################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/audio.c \
../Core/Src/ccsbcs.c \
../Core/Src/diskio.c \
../Core/Src/dwin.c \
../Core/Src/fatfs.c \
../Core/Src/ff.c \
../Core/Src/ff_gen_drv.c \
../Core/Src/fonts.c \
../Core/Src/game.c \
../Core/Src/input.c \
../Core/Src/keypad.c \
../Core/Src/main.c \
../Core/Src/st7735.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscall.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tcs3200.c \
../Core/Src/tm1637.c \
../Core/Src/user_diskio.c 

OBJS += \
./Core/Src/audio.o \
./Core/Src/ccsbcs.o \
./Core/Src/diskio.o \
./Core/Src/dwin.o \
./Core/Src/fatfs.o \
./Core/Src/ff.o \
./Core/Src/ff_gen_drv.o \
./Core/Src/fonts.o \
./Core/Src/game.o \
./Core/Src/input.o \
./Core/Src/keypad.o \
./Core/Src/main.o \
./Core/Src/st7735.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscall.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tcs3200.o \
./Core/Src/tm1637.o \
./Core/Src/user_diskio.o 

C_DEPS += \
./Core/Src/audio.d \
./Core/Src/ccsbcs.d \
./Core/Src/diskio.d \
./Core/Src/dwin.d \
./Core/Src/fatfs.d \
./Core/Src/ff.d \
./Core/Src/ff_gen_drv.d \
./Core/Src/fonts.d \
./Core/Src/game.d \
./Core/Src/input.d \
./Core/Src/keypad.d \
./Core/Src/main.d \
./Core/Src/st7735.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscall.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tcs3200.d \
./Core/Src/tm1637.d \
./Core/Src/user_diskio.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/audio.cyclo ./Core/Src/audio.d ./Core/Src/audio.o ./Core/Src/audio.su ./Core/Src/ccsbcs.cyclo ./Core/Src/ccsbcs.d ./Core/Src/ccsbcs.o ./Core/Src/ccsbcs.su ./Core/Src/diskio.cyclo ./Core/Src/diskio.d ./Core/Src/diskio.o ./Core/Src/diskio.su ./Core/Src/dwin.cyclo ./Core/Src/dwin.d ./Core/Src/dwin.o ./Core/Src/dwin.su ./Core/Src/fatfs.cyclo ./Core/Src/fatfs.d ./Core/Src/fatfs.o ./Core/Src/fatfs.su ./Core/Src/ff.cyclo ./Core/Src/ff.d ./Core/Src/ff.o ./Core/Src/ff.su ./Core/Src/ff_gen_drv.cyclo ./Core/Src/ff_gen_drv.d ./Core/Src/ff_gen_drv.o ./Core/Src/ff_gen_drv.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/game.cyclo ./Core/Src/game.d ./Core/Src/game.o ./Core/Src/game.su ./Core/Src/input.cyclo ./Core/Src/input.d ./Core/Src/input.o ./Core/Src/input.su ./Core/Src/keypad.cyclo ./Core/Src/keypad.d ./Core/Src/keypad.o ./Core/Src/keypad.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/st7735.cyclo ./Core/Src/st7735.d ./Core/Src/st7735.o ./Core/Src/st7735.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscall.cyclo ./Core/Src/syscall.d ./Core/Src/syscall.o ./Core/Src/syscall.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tcs3200.cyclo ./Core/Src/tcs3200.d ./Core/Src/tcs3200.o ./Core/Src/tcs3200.su ./Core/Src/tm1637.cyclo ./Core/Src/tm1637.d ./Core/Src/tm1637.o ./Core/Src/tm1637.su ./Core/Src/user_diskio.cyclo ./Core/Src/user_diskio.d ./Core/Src/user_diskio.o ./Core/Src/user_diskio.su

.PHONY: clean-Core-2f-Src

