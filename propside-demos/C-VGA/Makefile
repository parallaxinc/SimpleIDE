#
# Makefile for simple vga demo
# Written by Eric R. Smith, ersmith@totalspectrum.ca
# and placed in the public domain.
#

CC = propeller-elf-gcc
OBJCOPY = propeller-elf-objcopy
CFLAGS = -Os

all: vgademo.elf

vgademo.elf: vgademo.c vga.cog
	$(CC) -Os -o $@ $^

vga.cog: vga.c
	$(CC) -r -Os -mcog -o $@ $<
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@

clean:
	rm -rf *.o *.cog *.elf

run: all
	propeller-load -r vgademo.elf
