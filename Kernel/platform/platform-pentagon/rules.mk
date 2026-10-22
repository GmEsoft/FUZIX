#
#	ZX128 uses banked kernel images
#
export CCROSS_CCOPTS=-Os
export BANKED=-banked
export CCBANKED=-banked

CFLAGS += -Idev/zx/
#
# Tell the core code we are using the banked helpers
#
export BANKED=-banked
#
export CROSS_CC_SEG1=-Toverlay2
export CROSS_CC_SEG3=-Toverlay1
export CROSS_CC_SEG4=-Toverlay1
export CROSS_CC_SYS1=-Toverlay1
export CROSS_CC_SYS2=-Toverlay1
export CROSS_CC_SYS3=-Toverlay1
export CROSS_CC_SYS4=-Toverlay4
export CROSS_CC_SYS5=-Toverlay4
