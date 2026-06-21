ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
OUTPUT_PATH = $(ROOT_DIR)"/bin/rtcalc"
COMPILE_FLAGS = ""

compile:
	@if [ "$(origin OUTPUT_PATH)" = "file" ] && [ ! -d "bin/" ]; then \
		mkdir -p "bin/"; \
	fi

	@if ([ ! -d "bin/" ]); then mkdir "bin/"; fi
	clang $(ROOT_DIR)/src/rtcalc.c -I $(ROOT_DIR)/src/ -lm -std=gnu99 -o $(OUTPUT_PATH) $(COMPILE_FLAGS)

MISSING_PANDOC_MSG = "Missing dependency for making manpage \e[1m'pandoc'\e[0m.\nInstall it in order to form the manpage.\n"
MISSING_MANUAL_MD  = "Missing markdown manual file. Have you deleted it?\n"

gendoc:
	@# missing pandoc, aka what lets us form the page
	@if ( ! which pandoc > /dev/null 2>&1 ); then \
	    printf $(MISSING_PANDOC_MSG); \
		exit 1; \
	fi

	@# missing markdown file
	@if [ ! -f docs/rtcalc.md ]; then \
	    printf $(MISSING_MANUAL_MD); \
	    exit 1; \
	fi

	pandoc docs/rtcalc.md -s -t man -o docs/rtcalc.1
