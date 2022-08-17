HEAP_SIZE      = 8388208
STACK_SIZE     = 61800

PRODUCT = Tamagotchi.pdx

# Locate the SDK
SDK = ${PLAYDATE_SDK_PATH}
ifeq ($(SDK),)
	SDK = $(shell egrep '^\s*SDKRoot' ~/.Playdate/config | head -n 1 | cut -c9-)
endif

ifeq ($(SDK),)
$(error SDK path not found; set ENV value PLAYDATE_SDK_PATH)
endif

VPATH += src

# List C source files here
SRC = \
	$(wildcard src/*.c) \
	$(wildcard src/tamalib/*.c) \

# List all user directories here
UINCDIR = src/
UINCDIR += src/tamalib

include $(SDK)/C_API/buildsupport/common.mk

