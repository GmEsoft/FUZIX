CROSS_CCOPTS += -I../../dev/


CSRCS = devlpr.c devtty.c devfd.c devhd.c
CSRCS += devices.c main.c vt.c ide.c

DISCSRCS = discard.c

ASRCS = ubee.S crt0.S
ASRCS += tricks.S commonmem.S floppy.S

#DISCARD_DSRCS = ../../dev/devide_discard.c
DSRCS = ../../dev/blkdev.c ../../dev/mbr.c #../../dev/devide.c

NSRCS = ../../dev/net/net_native.c

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS)

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $< -o $@

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $< -o $@

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $< -o $@

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~
	rm -f fuzix.ss80 bootstrap.bin

image:
	fcc -mz80 -c bootstrap.S
	ldz80 -b bootstrap.o -o bootstrap.tmp
	dd if=bootstrap.tmp of=bootstrap.bin bs=128 skip=1
	dd if=/dev/zero of=fuzix.ss80 bs=512 count=800
	dd if=bootstrap.bin of=fuzix.ss80 conv=notrunc
	dd if=../../fuzix.bin of=fuzix.ss80 bs=512 seek=1 conv=notrunc

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Floppy disks
	# Boot floppy
	cp fuzix.ss80 $(IMAGES)
	# Test root floppy
	(cd ../../../Standalone/filesystem-src; ./build-mini-filesystem $(IMAGES)/root.ds82 64 1600 )
	# Make a blank disk image with partition
	# For the moment we make the disk bigger than the partition but it doesn't matter
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=512 count=62720 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cp $(IMAGES)/disk.img $(IMAGES)/disk.hd3
