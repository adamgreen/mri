#Copyright (C) 2011 by Sagar G V
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.*/
#
# Updates: 
#    Arthur Wolf & Adam Green in 2011-2012 - Updated to work with mbed.


#  Project Name
PROJECT=mri

# Output Object Directory
OUTDIR=obj

# Drop Directory
DROPDIR=../drop

# Final Target Archive
TARGET=$(OUTDIR)/$(PROJECT).ar

# List of sources to be compiled/assembled
CSRCS = $(wildcard *.c)
ASRCS =  $(wildcard *.S)

PLAT_CSRCS = platforms/architectures/cortex-m/cortex-m.c
PLAT_CSRCS += platforms/devices/lpc176x/lpc176x_uart.c
PLAT_CSRCS += platforms/devices/lpc176x/lpc176x.c
PLAT_CSRCS += platforms/boards/mbed1768/mbed1768.c
PLAT_CSRCS += platforms/semihost/semihost.c
PLAT_CSRCS += platforms/semihost/newlib/semihost_newlib.c
PLAT_CSRCS += platforms/semihost/mbed/semihost_mbed.c

PLAT_ASRCS = platforms/architectures/cortex-m/cortex-m_asm.S
PLAT_ASRCS += platforms/boards/mbed1768/mbed1768_asm.S
PLAT_ASRCS += platforms/semihost/newlib/newlib_asm.S


# List of the objects files to be compiled/assembled
OBJECTS = $(patsubst %.c,$(OUTDIR)/%.o,$(CSRCS)) $(patsubst %.S,$(OUTDIR)/%.o,$(ASRCS))
PLAT_OBJECTS = $(patsubst %.c,$(OUTDIR)/%.o,$(PLAT_CSRCS)) $(patsubst %.S,$(OUTDIR)/%.o,$(PLAT_ASRCS))

# Include path
INCDIRS = ../external/inc ../external/inc/LPC17xx . stubs

# DEFINEs to be used when building C/C++ code
DEFINES = -DTARGET_LPC1768 

# Optimization level
OPTIMIZATION = s

#  Compiler Options
GPFLAGS = -O$(OPTIMIZATION) -gstabs+3 -mcpu=cortex-m3 -mthumb -mthumb-interwork -fshort-wchar -ffunction-sections -fdata-sections -fpromote-loop-indices -Wall -Wextra -Wimplicit -Wcast-align -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -fno-exceptions
GPFLAGS += $(patsubst %,-I%,$(INCDIRS))
GPFLAGS += $(DEFINES)

GCFLAGS = -O$(OPTIMIZATION) -gstabs+3 -mcpu=cortex-m3 -mthumb -mthumb-interwork -fshort-wchar -ffunction-sections -fdata-sections -fpromote-loop-indices -Wall -Wextra -Werror -fno-exceptions
GCFLAGS += $(patsubst %,-I%,$(INCDIRS))
GCFLAGS += $(DEFINES)

LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -Wl,-Map=$(PROJECT).map,--cref,--gc-sections,--no-wchar-size-warning

ASFLAGS = $(LISTING) -mcpu=cortex-m3 -mthumb -x assembler-with-cpp
ASFLAGS += $(patsubst %,-I%,$(INCDIRS))

#  Compiler/Assembler/Linker Paths
GCC = arm-none-eabi-gcc
GPP = arm-none-eabi-g++
AS = arm-none-eabi-gcc
LD = arm-none-eabi-g++
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
REMOVE = rm
REMOVE_DIR = rm -r -f
COPY = cp
CAT = cat

# Switch to cs-rm on Windows.
ifeq "$(MAKE)" "cs-make"
REMOVE = cs-rm
REMOVE_DIR = rd /s /q
COPY = copy
CAT = type
endif

#########################################################################

all:: $(OUTDIR) $(TARGET) $(DROPDIR) $(DROPDIR)/mri.h $(DROPDIR)/mri.ar $(DROPDIR)/COPYING.LESSER

$(TARGET): $(OBJECTS) $(PLAT_OBJECTS)
	$(AR) -rc $@ $?

$(DROPDIR)/mri.h: mri.h version.h
	$(CAT) $? >$@

$(DROPDIR)/mri.ar:  $(TARGET)
	$(COPY) $? $@

$(DROPDIR)/COPYING.LESSER: COPYING.LESSER
	$(COPY) $? $@

$(OUTDIR):
	mkdir $(OUTDIR)

$(DROPDIR):
	mkdir $(DROPDIR)

clean:
	$(REMOVE_DIR) $(OUTDIR)
	$(REMOVE_DIR) $(DROPDIR)

#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

$(OUTDIR)/%.o : %.c
	mkdir -p $(dir $@)
	$(GCC) $(GCFLAGS) -c $< -o $@

$(OUTDIR)/%.o : %.S
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

#########################################################################