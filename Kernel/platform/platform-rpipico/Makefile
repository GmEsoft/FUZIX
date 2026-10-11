# Set one of these depending.
export PICO_SDK_FETCH_FROM_GIT = yes
#export PICO_SDK_PATH = ${HOME}/.pico-sdk/sdk/2.3.1

# Set 'SUBTARGET' to one of pico-sdk compatible boards
# valid values: pico or pico2
SUBTARGET ?= pico

ifeq ($(SUBTARGET),pico)
TOTALMEM=160
endif
ifeq ($(SUBTARGET),pico_w)
TOTALMEM=160
endif
ifeq ($(SUBTARGET),pico2)
TOTALMEM=320
endif
ifeq ($(SUBTARGET),pico2_w)
TOTALMEM=320
endif

PROCMEM ?= 64
FLASH_FIRMWARE_KIB ?= 96
FLASH_XIP_KIB ?= 0

PICOTOOLFLAGS= --family rp2040
ifneq ($(filter pico2 pico2_w,$(SUBTARGET)),)
PICOTOOLFLAGS= --family data
endif

PICOTOOL=picotool

ifeq (, $(shell which picotool))
PICOTOOL=build/_deps/picotool/picotool
endif

include ../../../version.mk

build/fuzix.elf: ../../version.c build/Makefile
	$(MAKE) -C build

../../version.c:
	$(MAKE) -C ../.. TARGET=rpipico version.c

# Reconfigure even when only a command-line memory/board setting changed.
build/Makefile: CMakeLists.txt $(wildcard ../../*.[chS]) $(wildcard ../../*/*.[chS]) FORCE
	mkdir -p build
	@if test -f build/CMakeCache.txt && ! grep -qx 'PICO_BOARD:STRING=${SUBTARGET}' build/CMakeCache.txt; then \
		rm -f build/CMakeCache.txt; \
		rm -rf build/CMakeFiles; \
	fi
	(cd build && cmake -DPICO_BOARD=${SUBTARGET} -DTOTALMEM=${TOTALMEM} -DPROCMEM=${PROCMEM} -DFLASH_FIRMWARE_KIB=${FLASH_FIRMWARE_KIB} -DFLASH_XIP_KIB=${FLASH_XIP_KIB} ..)

.PHONY: FORCE
FORCE:

image:: world
	@set -e; . ./build/flash-layout.env; \
	capacity=$$(../../../Standalone/mkftl -c -s $$FTL_SIZE_KIB -e $$FLASH_ERASE_BYTES -g $$FLASH_GC_RATIO); \
	if [ $$capacity -lt 128 ]; then echo "Flash filesystem region is too small" >&2; exit 1; fi; \
	fs_size=$$((capacity * 9 / 10)); \
	if [ $$fs_size -gt 2547 ]; then fs_size=2547; fi; \
	FSSIZE=$$fs_size ./update-flash.sh; \
	../../../Standalone/mkftl -s $$FTL_SIZE_KIB -e $$FLASH_ERASE_BYTES -g $$FLASH_GC_RATIO -o filesystem.ftl filesystem.img; \
	${PICOTOOL} uf2 convert filesystem.ftl -t bin filesystem.uf2 -o $$FTL_ADDRESS ${PICOTOOLFLAGS}

diskimage:

clean:
	rm -rf build
	rm -f uf2conv
	$(MAKE) -C ../../../Library/libs -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/util -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/V7/cmd -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/V7/cmd/sh -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/games -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/levee -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/cave -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/cursesgames -f Makefile.armm0 clean
	$(MAKE) -C ../../../Standalone clean
	$(MAKE) -C utils clean

world: build/fuzix.elf
	$(MAKE) -C ../../../Library/libs -f Makefile.armm0
	$(MAKE) -C ../../../Applications/util -f Makefile.armm0
	$(MAKE) -C ../../../Applications/V7/cmd -f Makefile.armm0
	$(MAKE) -C ../../../Applications/V7/cmd/sh -f Makefile.armm0
	$(MAKE) -C ../../../Applications/games -f Makefile.armm0
	$(MAKE) -C ../../../Applications/levee -f Makefile.armm0
	$(MAKE) -C ../../../Applications/cave -f Makefile.armm0
	$(MAKE) -C ../../../Applications/cursesgames -f Makefile.armm0
	$(MAKE) -C ../../../Standalone
	$(MAKE) -C utils
