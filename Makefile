#
# Makefile
#

include ../../Config.mk

OBJS	= main.o kernel.o

include $(CIRCLEHOME)/Rules.mk

INCLUDE := $(CIRCLE_STDLIB_INCLUDES) $(INCLUDE)
LIBS := $(CIRCLE_STDLIB_LIBS)
CPPFLAGS += -std=c++17

-include $(DEPS)

# Whisper / ggml include directories
INCLUDE += -I$(HOME)/whisper.cpp/include
INCLUDE += -I$(HOME)/whisper.cpp/ggml/include
INCLUDE += -I$(HOME)/whisper.cpp/ggml/src
