vpath %.c ../platform-smallz80
vpath %.S ../platform-smallz80

CROSS_CCOPTS += -I../../dev/ -I../platform-smallz80


CSRCS = devtty.c devices.c devrtc.c main.c

DISCSRCS = discard.c

ASRCS = smallz80.S crt0.S ide.S tricks.S commonmem.S map.S

DISCARD_DSRCS = ../../dev/tinyide_discard.c ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinyide.c
NSRCS =

DASRCS = 

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $< -o $@

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $< -o $@

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $< -o $@

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $< -o $@

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $< -o $@

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $@

$(DAOBJS): %.o: ../../dev/%.S
	$(CROSS_AS) $(ASOPTS) $@ $< -o $@

clean:
	rm -f $(OBJS) core *~ loader.bin *.tmp fuzix

loader.bin: loader.S
	fcc -mz80 -c $< -o loader.o
	ldz80 -b loader.o -o loader.tmp
	dd if=loader.tmp bs=256 skip=250 of=loader2.tmp
	dd if=loader2.tmp bs=1536 conv=sync of=loader.bin count=1

image: loader.bin
	$(CROSS_LD) -b -C 0x0100 -S 0xE000 -f CLDBbXSs -o ../../fuzix.bin \
	crt0.o commonmem.o smallz80.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../cpu-z80u/usermem_std-z80u.o map.o \
	../../usermem.o tricks.o main.o discard.o devrtc.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/bankfixed.o ../../devsys.o \
	devtty.o tinydisk.o tinydisk_discard.o \
	tinyide.o tinyide_discard.o ide.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image
	dd if=../platform-smallz80/partition.bin of=$(IMAGES)/disk.img bs=134217728 conv=sync count=1
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=3 conv=notrunc
	# Add the bootstrap
	dd if=loader.bin of=$(IMAGES)/disk.img conv=notrunc
	# Put the partition bits in
	dd if=../platform-smallz80/partition.bin seek=0 of=$(IMAGES)/disk.img bs=1 seek=446 skip=446 count=66 conv=notrunc
	# Make an emulator image of it. Geometry matters for this system
	cat ../platform-smallz80/idehdr.bin $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img

