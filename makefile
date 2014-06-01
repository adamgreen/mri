# Copyright 2014 Adam Green (https://github.com/adamgreen)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# User can set VERBOSE variable to have all commands echoed to console for debugging purposes.
ifdef VERBOSE
    Q :=
else
    Q := @
endif

# Make sure that gcov goal isn't used with incompatible rules.
ifeq "$(findstring gcov,$(MAKECMDGOALS))" "gcov"
    ifeq "$(findstring all,$(MAKECMDGOALS))" "all"
        $(error Can't use 'all' and 'gcov' goals together.)
    endif
    ifeq "$(findstring host,$(MAKECMDGOALS))" "host"
        $(error Can't use 'host' and 'gcov' goals together.)
    endif
endif

# *** High Level Make Rules ***
.PHONY : arm clean host all gcov

arm : ARM_BOARDS

host : RUN_CPPUTEST_TESTS RUN_CORE_TESTS

all : arm host

gcov : RUN_CPPUTEST_TESTS GCOV_CORE

clean : 
	@echo Cleaning MRI
	$Q $(REMOVE_DIR) $(OBJDIR) $(QUIET)
	$Q $(REMOVE_DIR) $(LIBDIR) $(QUIET)
	$Q $(REMOVE_DIR) $(GCOVDIR) $(QUIET)
	$Q $(REMOVE) mri_tests$(EXE) $(QUIET)
	$Q $(REMOVE) mri_tests_gcov$(EXE) $(QUIET)
	$Q $(REMOVE) CppUTest_tests$(EXE) $(QUIET)
	$Q $(REMOVE) CppUTest_tests_gcov$(EXE) $(QUIET)


#  Names of tools for cross-compiling ARMv7-M binaries.
ARMV7M_GCC := arm-none-eabi-gcc
ARMV7M_GPP := arm-none-eabi-g++
ARMV7M_AS  := arm-none-eabi-gcc
ARMV7M_LD  := arm-none-eabi-g++
ARMV7M_AR  := arm-none-eabi-ar

#  Names of tools for compiling binaries to run on this host system.
HOST_GCC := gcc
HOST_GPP := g++
HOST_AS  := gcc
HOST_LD  := g++
HOST_AR  := ar

# Handle Windows and *nix differences.
ifeq "$(OS)" "Windows_NT"
    MAKEDIR = mkdir $(subst /,\,$(dir $@))
    REMOVE := del /q
    REMOVE_DIR := rd /s /q
    QUIET := >nul 2>nul & exit 0
    EXE := .exe
else
    MAKEDIR = mkdir -p $(dir $@)
    REMOVE := rm
    REMOVE_DIR := rm -r -f
    QUIET := > /dev/null 2>&1 ; exit 0
    EXE :=
endif

# Flags to use when cross-compiling ARMv7-M binaries.
ARMV7M_GCCFLAGS := -Os -g3 -mcpu=cortex-m3 -mthumb -mthumb-interwork -Wall -Wextra -Werror -Wno-unused-parameter -MMD -MP
ARMV7M_GCCFLAGS += -ffunction-sections -fdata-sections -fno-exceptions -fno-delete-null-pointer-checks -fomit-frame-pointer
ARMV7M_GPPFLAGS := $(ARMV7M_GCCFLAGS) -fno-rtti
ARMV7M_GCCFLAGS += -std=gnu90
ARMV7M_LDFLAGS   = -mcpu=cortex-m3 -mthumb -Wl,-Map=$(basename $@).map,--cref,--gc-sections
ARMV7M_ASFLAGS  := -mcpu=cortex-m3 -mthumb -g -x assembler-with-cpp -MMD -MP

# Flags to use when compiling binaries to run on this host system.
HOST_GCCFLAGS := -O2 -g3 -Wall -Wextra -Werror -Wno-unused-parameter -MMD -MP
HOST_GCCFLAGS += -ffunction-sections -fdata-sections -fno-common
HOST_GCCFLAGS += -include CppUTest/include/CppUTest/MemoryLeakDetectorMallocMacros.h
HOST_GPPFLAGS := $(HOST_GCCFLAGS) -include CppUTest/include/CppUTest/MemoryLeakDetectorNewMacros.h
HOST_GCCFLAGS += -std=gnu90
HOST_ASFLAGS  := -g -x assembler-with-cpp -MMD -MP

# Output directories for intermediate object files.
OBJDIR        := obj
ARMV7M_OBJDIR := $(OBJDIR)/armv7-m
HOST_OBJDIR   := $(OBJDIR)/host

# Output directory for gcov files.
GCOVDIR := gcov

# Output directories for final libraries.
LIBDIR        := lib
ARMV7M_LIBDIR := $(LIBDIR)/armv7-m
HOST_LIBDIR   := $(LIBDIR)/host

# Modify some things if we want to run code coverage as part of this build.
ifeq "$(findstring gcov,$(MAKECMDGOALS))" "gcov"
    HOST_OBJDIR   := $(GCOVDIR)/$(HOST_OBJDIR)
    HOST_LIBDIR   := $(GCOVDIR)/$(HOST_LIBDIR)
    HOST_GCCFLAGS += -fprofile-arcs -ftest-coverage
    HOST_GPPFLAGS += -fprofile-arcs -ftest-coverage
    HOST_LDFLAGS  += -fprofile-arcs -ftest-coverage
    GCOV          := _gcov
else
    GCOV :=
endif

# Most of the needed headers are located here.
INCLUDES := include

# Start out with an empty header file dependency list.  Add module files as we go.
DEPS :=

# Useful macros.
objs = $(addprefix $2/,$(addsuffix .o,$(basename $(wildcard $1/*.c $1/*.cpp $1/*.S))))
armv7m_objs = $(call objs,$1,$(ARMV7M_OBJDIR))
host_objs = $(call objs,$1,$(HOST_OBJDIR))
add_deps = $(patsubst %.o,%.d,$(ARMV7M_$1_OBJ) $(HOST_$1_OBJ))
includes = $(patsubst %,-I%,$1)
define build_lib
	@echo Building $@
	$Q $(MAKEDIR)
	$Q $($1_AR) -rc $@ $?
endef
define link_exe
	@echo Building $@
	$Q $(MAKEDIR)
	$Q $($1_LD) $($1_LDFLAGS) $^ -o $@
endef


# MRI Core sources to build.
ARMV7M_CORE_OBJ := $(call armv7m_objs,core)
HOST_CORE_OBJ   := $(call host_objs,core)
HOST_CORE_LIB   := $(HOST_LIBDIR)/libmricore.a
$(HOST_CORE_LIB) : $(HOST_CORE_OBJ)
	$(call build_lib,HOST)
DEPS += $(call add_deps,CORE)

# Sources for newlib and mbed's LocalFileSystem semihosting support.
ARMV7M_SEMIHOST_OBJ := $(call armv7m_objs,semihost)
ARMV7M_SEMIHOST_OBJ += $(call armv7m_objs,semihost/newlib)
ARMV7M_SEMIHOST_OBJ += $(call armv7m_objs,semihost/mbed)
DEPS += $(call add_deps,SEMIHOST)

# Sources for ARMv7-M debug architecture.
ARMV7M_ARMV7M_OBJ := $(call armv7m_objs,architectures/armv7-m)
DEPS += $(call add_deps,ARMV7M)

# Native memory access sources.
ARMV7M_NATIVE_MEM_OBJ := $(call armv7m_objs,memory/native)
HOST_NATIVE_MEM_OBJ   := $(call host_objs,memory/native)
DEPS += $(call add_deps,NATIVE_MEM)

# LPC176x device sources.
ARMV7M_LPC176X_OBJ := $(call armv7m_objs,devices/lpc176x)
DEPS += $(call add_deps,LPC176X)


# mbed 1768 board
ARMV7M_MBED1768_OBJ := $(call armv7m_objs,boards/mbed1768)
ARMV7M_MBED1768_LIB = $(ARMV7M_LIBDIR)/libmri_mbed1768.a
$(ARMV7M_MBED1768_LIB) : INCLUDES := $(INCLUDES) boards/mbed1768 devices/lpc176x architecture/armv7-m cmsis/LPC17xx
$(ARMV7M_MBED1768_LIB) : $(ARMV7M_CORE_OBJ) $(ARMV7M_SEMIHOST_OBJ) $(ARMV7M_ARMV7M_OBJ) $(ARMV7M_NATIVE_MEM_OBJ) $(ARMV7M_LPC176X_OBJ) $(ARMV7M_MBED1768_OBJ)
	$(call build_lib,ARMV7M)
DEPS += $(call add_deps,MBED1768)

# All boards to be built for ARM target.
ARM_BOARDS : $(ARMV7M_MBED1768_LIB)


# Build CppUTest library which runs on host machine.
HOST_CPPUTEST_OBJ := $(call host_objs,CppUTest/src/CppUTest) $(call host_objs,CppUTest/src/Platforms/Gcc)
HOST_CPPUTEST_LIB := $(HOST_LIBDIR)/libCppUTest.a
$(HOST_CPPUTEST_LIB) : INCLUDES := $(INCLUDES) CppUTest/include
$(HOST_CPPUTEST_LIB) : $(HOST_CPPUTEST_OBJ)
	$(call build_lib,HOST)
DEPS += $(call add_deps,CPPUTEST)

# Unit tests for CppUTest library.
HOST_CPPUTEST_TESTS_OBJ := $(call host_objs,CppUTest/tests)
HOST_CPPUTEST_TESTS_EXE := CppUTest_tests$(GCOV)
$(HOST_CPPUTEST_TESTS_EXE) : INCLUDES := $(INCLUDES) CppUTest/include
$(HOST_CPPUTEST_TESTS_EXE) : $(HOST_CPPUTEST_TESTS_OBJ) $(HOST_CPPUTEST_LIB)
	$(call link_exe,HOST)
RUN_CPPUTEST_TESTS : $(HOST_CPPUTEST_TESTS_EXE)
	$Q $(HOST_CPPUTEST_TESTS_EXE)
DEPS += $(call add_deps,CPPUTEST_TESTS)

# Unit tests for MRI core library.
HOST_CORE_TESTS_OBJ := $(call host_objs,tests/tests) $(call host_objs,tests/mocks)
HOST_CORE_TESTS_EXE := mri_tests$(GCOV)
$(HOST_CORE_TESTS_EXE) : INCLUDES := $(INCLUDES) CppUTest/include tests/tests tests/mocks
$(HOST_CORE_TESTS_EXE) : $(HOST_CORE_TESTS_OBJ) $(HOST_NATIVE_MEM_OBJ) $(HOST_CORE_LIB) $(HOST_CPPUTEST_LIB)
	$(call link_exe,HOST)
RUN_CORE_TESTS : $(HOST_CORE_TESTS_EXE)
	$Q $(HOST_CORE_TESTS_EXE)
DEPS += $(call add_deps,CORE_TESTS)

GCOV_CORE : RUN_CORE_TESTS
	$Q $(REMOVE mri_output.txt $(QUIET)
	$Q mkdir -p gcov/mri_tests
	$(foreach i, $(HOST_CORE_OBJ),$(shell gcov -object-directory=$(dir $i) $(notdir $i) >> mri_output.txt))
	$Q mv mri_output.txt gcov/mri_tests/
	$Q mv *.gcov gcov/mri_tests/
	$Q CppUTest/scripts/filterGcov.sh gcov/mri_tests/mri_output.txt /dev/null gcov/mri_tests/mri.txt
	$Q cat gcov/mri_tests/mri.txt


# *** Pattern Rules ***
$(ARMV7M_OBJDIR)/%.o : %.c
	@echo Compiling $<
	$Q $(MAKEDIR)
	$Q $(ARMV7M_GCC) $(ARMV7M_GCCFLAGS) $(call includes,$(INCLUDES)) -c $< -o $@

$(ARMV7M_OBJDIR)/%.o : %.S
	@echo Assembling $<
	$Q $(MAKEDIR)
	$Q $(ARMV7M_AS) $(ARMV7M_ASFLAGS) $(call includes,$(INCLUDES)) -c $< -o $@

$(HOST_OBJDIR)/%.o : %.c
	@echo Compiling $<
	$Q $(MAKEDIR)
	$Q $(HOST_GCC) $(HOST_GCCFLAGS) $(call includes,$(INCLUDES)) -c $< -o $@

$(HOST_OBJDIR)/%.o : %.cpp
	@echo Compiling $<
	$Q $(MAKEDIR)
	$Q $(HOST_GPP) $(HOST_GPPFLAGS) $(call includes,$(INCLUDES)) -c $< -o $@


# *** Pull in header dependencies if not performing a clean build. ***
ifneq "$(findstring clean,$(MAKECMDGOALS))" "clean"
    -include $(DEPS)
endif
