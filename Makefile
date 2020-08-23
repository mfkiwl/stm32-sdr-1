# the executable file to be built
EXEC=main.elf
BINARY=main.bin
TARGET=STM32F401xE
# the location of STM32FxCubeMX
CUBEMX=/home/knightshrub/.local/stm32/STM32Cube_FW_F4_V1.25.0

# ARM CMSIS library
CMSIS=$(CUBEMX)/Drivers/CMSIS
CMSIS_DEVICE=$(CMSIS)/Device/ST/STM32F4xx

# the device linker script
LDSCRIPT=STM32F401RETx_FLASH.ld
# the startup file
STARTUP=$(CMSIS_DEVICE)/Source/Templates/gcc/startup_stm32f401xe.s

# source file directory
SRCDIR=src
# header file directory
INCDIR=include
# object file directory
OBJDIR=obj
# the list of header files to be watched
HEADERS=$(wildcard $(INCDIR)/*.h)
# the list of source files to be watched
SOURCES=$(wildcard $(SRCDIR)/*.c)
# the list of object files to be generated
OBJECTS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# startup code
STARTUP_OBJ=$(patsubst %.s,$(OBJDIR)/%.o,$(notdir $(STARTUP)))

# the needed toolchain binaries
CC=arm-none-eabi-gcc
GDB=arm-none-eabi-gdb
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size
# compiler search path
INC=-I $(INCDIR) \
	-I $(CMSIS)/Include \
	-I $(CMSIS_DEVICE)/Include \
    -I $(CMSIS)/DSP/Include
# additional linker searchpath for ARM math library
LINC=-L $(CMSIS)/Lib/GCC
# compiler flags
CFLAGS=$(INC)
CFLAGS+=-DARM_MATH_CM4
CFLAGS+=-D$(TARGET) -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS+=-Wall -Werror -std=c99
CFLAGS+=-O2 -ggdb -ffast-math -ffreestanding -ffunction-sections -fdata-sections -flto
# linker flags
LDFLAGS=-Wl,--gc-sections -T $(LDSCRIPT)
LDFLAGS+=$(LINC)
LDLIBS=-Wl,-larm_cortexM4lf_math

.PHONY: all clean debug prog

all: $(EXEC)
	$(SIZE) $(EXEC)

$(BINARY): $(EXEC)
	$(OBJCOPY) -O binary $< $@

$(EXEC): $(STARTUP_OBJ) $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(STARTUP_OBJ): $(STARTUP)
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# first start openocd like this:
# openocd -f interface/stlink-v2-1.cfg -f target/stm32f4x.cfg
debug: $(EXEC)
	$(GDB) -ex "target remote :3333" $<

prog: $(BINARY)
	st-flash --reset write $< 0x8000000

clean:
	rm -rf $(OBJDIR) $(EXEC) $(BINARY)
