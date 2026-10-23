ASRCS = crt0.S tricks.S commonmem.S vz200.S video.S
CSRCS = devices.c main.c devtty.c sd.c devlpr.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinysd.c
DASRCS =
NSRCS = 

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DHOBJS = $(patsubst ../../dev/%.c,%.o, $(DHSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DHOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

JUNK = *.o *.bin *.tmp

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $@

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG4) -c $< -o $@

$(DAOBJS): %.o: ../../dev/%.S
	$(CROSS_AS) $(ASOPTS) $<  -o $@

$(DISCARD_COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $< -o $@

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $< -o $@

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.boot booter.tmp

font:
	ldz80 -b -D 0x0000 ../../font/font8x8.o -o font

image: booter boot bootblock font
	$(CROSS_LD) -b -C 0x9000 -O 0x4000 -X 0x7200 -f bXSsCLDB -o ../../fuzix.bin \
	crt0.o commonmem.o vz200.o devlpr.o devtty.o video.o sd.o \
	../../start.o ../../version.o ../../cpu-z80u/lowlevel-z80u-banked.o \
	../../cpu-z80u/usermem_std-z80u-banked.o \
	../../usermem.o tricks.o main.o discard.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/simple.o ../../devsys.o ../../vt.o \
	tinydisk.o tinydisk_discard.o tinysd.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap -R fuzix.relocs
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	# Perform the banking conversion
	bankman ../../fuzix.bin fuzix.tmpmap fuzix.relocs z80
	# TODO: font
	# Assemble the ROM as a single image
	# Our image consists of the following in order starting at LBA 1
	# Bank 0 0x4000 for 10K
	# Bank 1 0x4000 for 10K
	# Bank 0 0x9000 for 28K
	# Common 0x7200 for 5.5K
	# Font packed on the end for machines with the high res mod
	dd if=../../fuzix.bin of=fuzix.boot bs=512 skip=160 count=20
	dd if=../../fuzix.bin of=fuzix.boot bs=512 skip=288 seek=20 count=20
	dd if=../../fuzix.bin of=fuzix.boot bs=512 skip=72 seek=40 count=56
	dd if=../../fuzix.bin of=fuzix.boot bs=512 skip=57 seek=96 count=11
	dd if=font of=fuzix.boot bs=512 seek=107 count=4

booter: booter.S
	fcc -mz80 -c booter.S
	ldz80 -b booter.o -o booter.tmp
	dd if=booter.tmp of=booter.vz bs=1 skip=32744 count=536
#
#	Alternate Boot ROM image.
#	TODO: support bootstrapping sanely off the standard boot rom with a .VZ
#
boot: boot.S
	fcc -mz80 -c boot.S
	ldz80 -b boot.o -o boot.tmp
	dd if=boot.tmp bs=1 skip=16384 >boot.tmp2
	dd if=boot.tmp2 bs=6034 count=1 conv=sync >sdboot.rom

bootblock: bootblock.S
	fcc -mz80 -c bootblock.S
	ldz80 -b bootblock.o -o bootblock.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel extra blocks
	dd if=fuzix.boot of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Add the boot block at the start
	dd if=bootblock.bin bs=1 skip=34816 count=384 of=$(IMAGES)/disk.img conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	# Loader
	cp booter.vz $(IMAGES)/booter.vz
	cp sdboot.rom $(IMAGES)/sdboot.rom
