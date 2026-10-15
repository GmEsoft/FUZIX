#
#	Build the esp8266 valaddr hardware test into a Fuzix user binary.
#
#		FUZIX_ROOT=/path/to/FUZIX make
#
#	The resulting 'check' binary can be put into a flash filesystem
#	image with Standalone/ucp (see ../README.md).
#

FUZIX_ROOT ?= ../../../../..
USERCPU = esp8266

include $(FUZIX_ROOT)/Target/rules.$(USERCPU)

all: check

check: check.o
	$(LINKER) $(CRT0) check.o -o $@ $(LINKER_OPT) $(LINKER_TAIL)

clean:
	rm -f check check.o *.bin *.bin.data *.bin.text

.PHONY: all clean
