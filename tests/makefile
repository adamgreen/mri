#Set this to @ to keep the makefile quiet
SILENCE = @

#---- Outputs ----#
COMPONENT_NAME = mri
TARGET_LIB = lib/lib$(COMPONENT_NAME).a
	
TEST_TARGET = $(COMPONENT_NAME)_tests

#--- Inputs ----#
PROJECT_HOME_DIR = .
CPPUTEST_HOME = ../CppUTest

CPP_PLATFORM = Gcc

CPPUTEST_CPPFLAGS += -fno-common
CPPUTEST_CPPFLAGS += -DTARGET_LPC1768
CPPUTEST_WARNINGFLAGS += -Wall 
CPPUTEST_WARNINGFLAGS += -Werror 
CPPUTEST_WARNINGFLAGS += -Wswitch-default 
CPPUTEST_WARNINGFLAGS += -Wswitch-enum
CPPUTEST_WARNINGFLAGS += -Wno-unused-parameter
CPPUTEST_WARNINGFLAGS += -Wno-overlength-strings
CPPUTEST_CFLAGS += -std=c99
CPPUTEST_CFLAGS += -Wextra 
CPPUTEST_CFLAGS += -Wstrict-prototypes

SRC_DIRS =                                      \
	../mri                                      \

TEST_SRC_DIRS = \
	tests \
	mocks \
	
INCLUDE_DIRS =\
  $(CPPUTEST_HOME)/include          \
  mocks                             \
  tests                             \
  ../external/inc                   \
  ../external/inc/LPC17xx           \
  ../mri                            \

include $(CPPUTEST_HOME)/build/MakefileWorker.mk
