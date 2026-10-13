# Fuzix for the 'SmallZ180' from stack180.com

## Supported Hardware

- SmallZ180
- Optional dual serial card/floppy

## Unsupported

Floppy interface

## Installation

make diskimage produces a bootable disk.img file that is a raw image
suitable for writing to disk.

Boot with 'G' return, as with CP/M

## Memory Map

The Z180 version thankfully has a flat 1MB map space. For the Z180 port
we use almost identical code shared with the Z80 version but replace the
mapping routines and tricks file. The rest is entirely shared.

## TODO

Use the Z180 MMU for fork copies and maybe some other oddments. It's awkward
to share all the code and use the general Z180 style port as it assumes that
you want to use the internal I/O heavily whereas this platform really
doesn't make much use of the CPU features at all.
