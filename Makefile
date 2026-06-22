ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
OUTPUT_PATH = $(ROOT_DIR)"/bin/rtcalc"
ASM_FLAGS = -f elf64 -g -F dwarf -I $(ROOT_DIR)/src/

all: build
	@# Default: build from assembly sources

build:
	@if [ ! -d "build/" ]; then mkdir "build/"; fi
	@if ([ ! -d "bin/" ]); then mkdir "bin/"; fi
	nasm $(ASM_FLAGS) $(ROOT_DIR)/src/constants.asm -o $(ROOT_DIR)/build/constants.o
	nasm $(ASM_FLAGS) $(ROOT_DIR)/src/utils.asm     -o $(ROOT_DIR)/build/utils.o
	nasm $(ASM_FLAGS) $(ROOT_DIR)/src/evaluate.asm  -o $(ROOT_DIR)/build/evaluate.o
	nasm $(ASM_FLAGS) $(ROOT_DIR)/src/color.asm     -o $(ROOT_DIR)/build/color.o
	nasm $(ASM_FLAGS) $(ROOT_DIR)/src/rtcalc.asm    -o $(ROOT_DIR)/build/rtcalc.o
	clang $(ROOT_DIR)/build/constants.o \
	      $(ROOT_DIR)/build/utils.o \
	      $(ROOT_DIR)/build/evaluate.o \
	      $(ROOT_DIR)/build/color.o \
	      $(ROOT_DIR)/build/rtcalc.o \
	      -o $(OUTPUT_PATH) -lm -no-pie

clean:
	rm -rf build/ bin/

MISSING_PANDOC_MSG = "Missing dependency for making manpage \e[1m'pandoc'\e[0m.\nInstall it in order to form the manpage.\n"
MISSING_MANUAL_MD  = "Missing markdown manual file. Have you deleted it?\n"

gendoc:
	@if ( ! which pandoc > /dev/null 2>&1 ); then \
	    printf $(MISSING_PANDOC_MSG); \
		exit 1; \
	fi

	@if [ ! -f docs/rtcalc.md ]; then \
	    printf $(MISSING_MANUAL_MD); \
	    exit 1; \
	fi

	pandoc docs/rtcalc.md -s -t man -o docs/rtcalc.1
