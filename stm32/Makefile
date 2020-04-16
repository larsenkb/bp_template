TARGET		= main

# debug build?
DEBUG	= 1
OPT		= -Og
#OPT		:= -Os -g

# Add inputs and outputs from these tool invocations to the build variables 
#A_SRCS = $(wildcard *.s)
LIB_A_SRCS = $(wildcard *.s)

#C_SRCS = $(wildcard *.c)
LIB_C_SRCS = $(wildcard *.c)

#CPP_SRCS =
LIB_CPP_SRCS = $(wildcard *.cpp)

LIB_OBJS	 = $(LIB_A_SRCS:.s=.o)
LIB_OBJS	+= $(LIB_C_SRCS:.c=.o)
LIB_OBJS	+= $(LIB_CPP_SRCS:.cpp=.o)

DEPS	= $(patsubst %.c,%.d,$(LIB_C_SRCS))

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

C_INCLUDES	= -I.

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

LDLIBS		+= -specs=nano.specs
#LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LDLIBS		+= -Wl,--start-group  -lm -lnosys -Wl,--end-group

LDFLAGS	+= --static -nostartfiles
LDFLAGS	+= -T$(LDSCRIPT)
LDFLAGS	+= $(ARCH_FLAGS)
LDFLAGS	+= -Wl,-Map=$(*).map
LDFLAGS	+= -Wl,--gc-sections

bplib = libbp.a

.SUFFIXES:	.elf .bin .hex .srec .list .map .images
.SECONDEXPANSION:
.SECONDARY:

all: $(bplib) 

#$(TARGET).elf: $(DEPS) $(alib) $(OBJS) Makefile

#elf:	$(DEPS) $(TARGET).elf


$(bplib): $(LIB_OBJS)
	$(AR) rcs $@ $^

#.PHONY: libfoo
#libfoo:
#	$(MAKE) -C stm32

print-%:
	@echo $*=$($*)



%.o: %.c
	$(CC) -c $(CFLAGS) -MD $< -o $@
#$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(notdir $(<:.c=.lst)) $< -o $@

%.o: %.cxx
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cxx

%.o: %.cpp
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cpp

%.o: %.s
	$(AS) -c $(ASFLAGS) $< -o $@

clean:
	$(RM) $(LIB_OBJS) $(LIB_OBJS:.o=.d)


.PHONY: clean


