#!/bin/sh
./imgpacker ../data/testimage.png image_rom.bin ../vhdl/palette.vhdl
romswak data image_rom.bin -width 8 -mif -o ../data/image_rom.mif
rm image_rom.bin
