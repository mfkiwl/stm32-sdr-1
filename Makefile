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
# compiler flags
CFLAGS=$(INC)
CFLAGS+=-D$(TARGET) -mcpu=cortex-m4 -mthumb
CFLAGS+=-Wall -Werror -std=c99
CFLAGS+=-Os -ggdb -ffreestanding -ffunction-sections -fdata-sections -flto
# linker flags
LDFLAGS=-Wl,--gc-sections -T $(LDSCRIPT)

all: dirs $(EXEC) $(BINARY)
	$(SIZE) $(EXEC)

$(BINARY): $(EXEC)
	$(OBJCOPY) -O binary $< $@

$(EXEC): $(STARTUP_OBJ) $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(STARTUP_OBJ): $(STARTUP)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $@

dirs: $(OBJDIR) 

.PHONY: clean debug prog

debug: $(EXEC)
	$(GDB) -ex "target remote :4242" $<

prog: $(BINARY)
	st-flash write $< 0x8000000

clean:
	rm -rf $(OBJECTS) $(EXEC) $(BINARY)
