#
#	Banked kernel images
#
#
# Tell the core code we are using the banked helpers
#
export CROSS_CCOPTS+=-Os
export BANKED=-banked
export CCBANKED=-banked
#
export CROSS_CC_SEG1=
export CROSS_CC_SEG3=-Toverlay1
export CROSS_CC_SEG4=-Toverlay2
export CROSS_CC_SYS1=-Toverlay1
export CROSS_CC_SYS2=-Toverlay1
export CROSS_CC_SYS3=-Toverlay1
export CROSS_CC_SYS4=-Toverlay1
export CROSS_CC_SYS5=-Toverlay2
export CROSS_CC_SEGDISC=-Tdiscard
export CROSS_CC_VIDEO=-Toverlay1
export CROSS_CC_VIDEO=-Tcode

