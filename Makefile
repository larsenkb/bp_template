TARGET		= main

# debug build?
DEBUG	= 1
OPT		= -Og
#OPT		:= -Os -g

# Add inputs and outputs from these tool invocations to the build variables 
A_SRCS = $(wildcard *.s)

C_SRCS  = $(wildcard *.c)
C_SRCS += $(wildcard onewire/*.c)

OBJS	 = $(A_SRCS:.s=.o)
OBJS	+= $(C_SRCS:.c=.o)

LIB_SRCS = $(wildcard stm32/*.c)
LIB_SRCS += $(wildcard stm32/*.s)

DEPS	= $(patsubst %.c,%.d,$(C_SRCS))

# fpu
# NONE for Cortex-M0/M0+/M3
FPU =

# float-abi
FLOAT-ABI	?=	-mfloat-abi=soft

CPU  = -mcpu=cortex-m3

MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

AS_DEFS		=
AS_INCLUDES	=

AS_FLAGS	= $(MCU) $(AS_DEFS) $(AS_INCLUDES) -mthumb -mcpu=cortex-m3

PREFIX	?= arm-none-eabi-
CC		:= $(PREFIX)gcc
CXX		:= $(PREFIX)g++
LD		:= $(PREFIX)gcc
AR		:= $(PREFIX)ar
AS		:= $(PREFIX)as
OBJCOPY	:= $(PREFIX)objcopy
SZ		:= $(PREFIX)size
OBJDUMP	:= $(PREFIX)objdump
GDB		:= $(PREFIX)gdb
STFLASH = $(shell which st-flash)

LDSCRIPT	?= bluepill.ld

CSTD	?= -std=c99

C_INCLUDES	= -I. -Istm32 -Ionewire

C_DEFS		+= -DSTM32F1
C_DEFS		+= -DSTM32F103C8Tx
C_DEFS		+= -DSTM32
C_DEFS		+= -DUSE_STDPERIPH_DRIVER
C_DEFS		+= -DDEBUG
C_DEFS		+= -DSTM32F10X_MD
C_DEFS		+= -D__CM3_REV=0x0500


CFLAGS		= $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS		+= -g -gdwarf-2
endif

bplib = libbp

LDLIBS		+= -specs=nano.specs
#LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LDLIBS		+= -Wl,--start-group -Lstm32 -lbp -lc -lm -lnosys -Wl,--end-group
#LDLIBS		+= $(alib) -lc -lm -lnosys

LDFLAGS	+= --static -nostartfiles
LDFLAGS	+= -T$(LDSCRIPT)
LDFLAGS	+= $(ARCH_FLAGS)
LDFLAGS	+= -Wl,-Map=$(*).map
LDFLAGS	+= -Wl,--gc-sections


.SUFFIXES:	.elf .bin .hex .srec .list .map .images
.SECONDEXPANSION:
.SECONDARY:

all: $(TARGET).elf 

$(TARGET).elf:  stm32/$(bplib).a $(OBJS) $(C_SRCS) Makefile

#$(TARGET).elf:	$(DEPS) $(TARGET).elf

stm32/$(bplib).a: $(LIB_SRCS)
	$(MAKE) -C stm32
	$(AR) rcs $@ $^

print-%:
	@echo $*=$($*)

%.bin: %.elf
	$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.hex: %.elf
	$(OBJCOPY) -Oihex $(*).elf $(*).hex

%.srec: %.elf
	$(OBJCOPY) -Osrec $(*).elf $(*).srec

%.list: %.elf
	$(OBJDUMP) -S $(*).elf > $(*).list

%.elf: $(OBJS) $(LDSCRIPT) Makefile stm32/$(bplib).a
	$(LD) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@
	$(SZ) $@

# add -MD to create *.d file
%.o: %.c %.h
	$(CC) -c $(CFLAGS)  $< -o $@

%.o: %.cxx
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cxx

%.o: %.cpp
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cpp

%.o: %.s
	$(AS) -c $(ASFLAGS) $< -o $@

ctags:
	ctags *.[chsS] inc/*.[chs] src/*.[chs] 

clean: 
	$(RM) $(OBJS) $(OBJS:.o=.d) $(TARGET).elf $(TARGET).bin $(TARGET).map 
	$(MAKE) -C stm32 clean

#clobber: clean
#	rm -f *.elf *.bin *.hex *.srec *.list *.lst *.map $(CLOBBER)

flash: $(TARGET).bin
	$(STFLASH) --reset $(FLASHSIZE) write $(TARGET).bin 0x8000000

flashit: $(TARGET).bin
	@echo "Flashing...$(TARGET).bin"
	openocd -f target/bluepillst.cfg -c "flashit $(TARGET).bin"
#	openocd -f target/bluepillst.cfg -c init -c "reset halt" -c "flash write_image erase $(TARGET).bin 0x8000000" -c "verify_image $(TARGET).bin" -c reset -c shutdown
#openocd -f openocd.cfg -c "program $(TARGET).elf verify reset" -c shutdown


.PHONY: libfoo flashit flash
libfoo:
	$(MAKE) -C stm32

.PHONY: images clean elf bin hex srec list all ctags

-include $(OBJS:.o=.d)


# Each subdirectory must supply rules for building sources it contributes
#%.o: %.c
#	@echo 'Building file: $<'
#	@echo 'Invoking: MCU GCC Compiler'
#	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32F1 -DSTM32F103C8Tx -DSTM32 -DUSE_STDPERIPH_DRIVER -DDEBUG -DSTM32F10X_MD -I . -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
#	@echo 'Finished building: $<'
#	@echo ' '


#%.o: %.s
#	@echo 'Building file: $<'
#	@echo 'Invoking: MCU GCC Assembler'
#	arm-none-eabi-as -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -g -o "$@" "$<"
#	@echo 'Finished building: $<'
#	@echo ' '



