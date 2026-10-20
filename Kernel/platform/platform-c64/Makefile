
CSRCS = devtty.c
CSRCS += devices.c main.c

ASRCS = c64.S crt0.S
ASRCS += tricks.S commonmem.S

DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c
DISCARD_DSRCS = ../../dev/tinyide_discard.c ../../dev/tinydisk_discard.c

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=$(BINEXT))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $< -Toverlay4

$(AOBJS): %$(BINEXT): %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

$(DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c -Toverlay1 $<
	mv ../../dev/$*.o .

$(DISCARD_DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<
	mv ../../dev/$*.o .

clean:
	rm -f $(OBJS) *.o core *~ fuzix.img fuzix.bin

romload: romload.S
	fcc -c -m6502 romload.S
	ld6502 -b romload.o -o romload.tmp
	dd if=romload.tmp bs=1024 skip=40 of=romload
	
image:	romload
	$(CROSS_LD) -b -f CSsLDBbX -S 0x0800 -Z 0x00 -O 0x8000 -C 0xB000 -X 0x4000 \
	-o ../../fuzix.bin -m fuzix.tmpmap -R fuzix.relocs \
	crt0.o commonmem.o c64.o ../../start.o ../../version.o ../../cpu-6502/lowlevel-6502.o \
	tricks.o main.o ../../timer.o ../../kdata.o devices.o \
	tinydisk.o tinydisk_discard.o tinyide.o tinyide_discard.o ../../mm/memalloc_none.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../swap.o ../../mm/simple.o \
	../../tty.o ../../devsys.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_exec16.o ../../syscall_exec.o ../../blk512.o ../../vt.o \
	../../usermem.o ../../cpu-6502/usermem_std-6502.o devtty.o /opt/fcc/lib/6502/lib6502.a
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	bankman ../../fuzix.bin fuzix.tmpmap fuzix.relocs 6502
	# Now assemble an image in cartridge layout
	# Copy common from B000 into a temporary
	dd if=../../fuzix.bin of=fuzix.shared bs=4096 skip=11 count=1
	# Pack ROM bank 1
	# Insert first 8K block (0000-1FFF)
	dd if=../../fuzix.bin bs=8192 seek=0 skip=0 count=1 of=fuzix.rom
	# Insert leading boot area (0x8000 in our image to 0x0000)
	dd if=../../fuzix.bin bs=256 seek=0 skip=128 count=1 of=fuzix.rom conv=notrunc
	# Insert high block 4000-5FFF
	dd if=../../fuzix.bin bs=8192 seek=1 skip=2 count=1 of=fuzix.rom
	# 8000-BFFF from overlay0 into cartridge bank 1
	dd if=../../fuzix.bin bs=8192 skip=12 seek=2 count=2 of=fuzix.rom
	# 8000-BFFF from overlay1 into cartridge bank 2
	dd if=../../fuzix.bin bs=8192 skip=20 seek=4 count=2 of=fuzix.rom
	# 8000-BFFF from overlay2 into cartridge bank 3
	dd if=../../fuzix.bin bs=8192 skip=28 seek=6 count=2 of=fuzix.rom
	# 8000-BFFF from overlay3 into cartridge bank 4
	dd if=../../fuzix.bin bs=8192 skip=36 seek=8 count=2 of=fuzix.rom
	# Insert the shared code into each bank
	dd if=fuzix.shared of=fuzix.rom bs=1024 seek=28 conv=notrunc
	dd if=fuzix.shared of=fuzix.rom bs=1024 seek=44 conv=notrunc
	dd if=fuzix.shared of=fuzix.rom bs=1024 seek=60 conv=notrunc
	dd if=fuzix.shared of=fuzix.rom bs=1024 seek=76 conv=notrunc
	# And size out. We've filled 5 banks of 8
	dd if=/dev/zero of=fuzix.rom bs=16384 seek=5 count=3
	
IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the bootstrap loader
	# dd if=loader.bin of=$(IMAGES)/disk.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	# And the cartridge (use the .rom file or VICE seems to break ?)
	#cartconv -t ide64 -i fuzix.rom -o $(IMAGES)/fuzix.crt
	cp fuzix.rom $(IMAGES)


