# the executable file to be built
EXEC=main.elf
BINARY=main.bin
TARGET=STM32F401xE
# the location of STM32FxCubeMX
CUBEMX=$(HOME)/.local/stm32/STM32Cube_FW_F4_V1.25.0

# ARM CMSIS library
CMSIS=$(CUBEMX)/Drivers/CMSIS
CMSIS_DEVICE=$(CMSIS)/Device/ST/STM32F4xx

# the device linker script
# LDSCRIPT=STM32F401RETx_FLASH.ld
#LDSCRIPT=$(CUBEMX)/Projects/STM32F401RE-Nucleo/Templates_LL/SW4STM32/NUCLEO-F401RE/STM32F401RETx_FLASH.ld
LDSCRIPT=stm32f401ret6.ld
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
OBJECTS+=$(STARTUP_OBJ)


# the needed toolchain binaries
COMPILER ?= arm-none-eabi-gcc
CC = $(COMPILER)

GDB=arm-none-eabi-gdb
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size

# define the architecture specifics
ARCH_CFLAGS=-mcpu=cortex-m4 \
            -mfloat-abi=hard \
            -mfpu=fpv4-sp-d16 \
            -mthumb

# compiler search path
CINC=-I $(INCDIR) \
	-I $(CMSIS)/Include \
	-I $(CMSIS_DEVICE)/Include \
    -I $(CMSIS)/DSP/Include

# additional linker searchpaths
LINC=-L $(CMSIS)/Lib/GCC


CORTEXM_SYSROOT=$(shell arm-none-eabi-gcc $(ARCH_CFLAGS) -print-sysroot)
CORTEXM_MULTILIB=$(shell arm-none-eabi-gcc $(ARCH_CFLAGS) -print-multi-directory)
CORTEXM_LIBGCC_DIR=$(dir $(shell arm-none-eabi-gcc $(ARCH_CFLAGS) -print-libgcc-file-name))

LIBGCC_OBJS = $(wildcard $(CORTEXM_LIBGCC_DIR)*.o)
OBJECTS+=$(LIBGCC_OBJS)

ifneq ('', '$(findstring gcc,$(shell $(CC) --version))')
    $(info ========== arm-none-eabi-gcc ==========)
    CFLAGS=-Os -nostdlib
endif

ifneq ('', '$(findstring clang,$(shell $(CC) --version))')
    $(info ========== clang ==========)
    CFLAGS=-Qunused-arguments \
           --target=arm-none-eabi \
           --sysroot=$(CORTEXM_SYSROOT) \
           -rtlib=libgcc \
           -nostdlib

    CFLAGS+=-Oz

    LINC+=-L$(CORTEXM_LIBGCC_DIR) \
          -L$(CORTEXM_SYSROOT)/lib/$(CORTEXM_MULTILIB)
endif

# compiler flags
CDEF=-DARM_MATH_CM4 \
     -D$(TARGET) 

CFLAGS+=$(CINC) $(CDEF) $(ARCH_CFLAGS)

CFLAGS+=-Wall \
        -Werror \
        -std=c99
CFLAGS+=-ggdb
CFLAGS+=-ffast-math \
        -ffreestanding \
        -ffunction-sections \
        -fdata-sections \
        -flto
# linker flags
LDFLAGS=-Wl,--gc-sections \
        -T $(LDSCRIPT)
LDFLAGS+=$(LINC)
LDLIBS=-Wl,-larm_cortexM4lf_math \
       -Wl,-lm \
       -Wl,-lgcc \
       -Wl,-lc

.PHONY: all clean debug prog

define src-to-obj
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
endef

all: $(EXEC)

$(BINARY): $(EXEC)
	$(OBJCOPY) -O binary $< $@

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	$(SIZE) $(EXEC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(src-to-obj)

$(STARTUP_OBJ): $(STARTUP)
	$(src-to-obj)

debug: $(EXEC)
	$(GDB) -ex 'target extended-remote | openocd -c "gdb_port pipe; log_output openocd.log" -f interface/stlink.cfg -f target/stm32f4x.cfg' $<

prog: $(BINARY)
	st-flash --reset write $< 0x8000000

clean:
	rm -rf $(OBJDIR) $(EXEC) $(BINARY)
