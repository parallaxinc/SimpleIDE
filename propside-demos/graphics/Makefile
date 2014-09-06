# #########################################################
# This makefile allows building and cleaning the Graphics Demo
#
# To build with LMM model, enter $ make clean; make
# Currently xmm and xmmc modes are not supported.
#
# Copyright (c) 2011 Parallax, Inc.
# Code by Steve Denson
# All rights MIT licensed
# #########################################################

# Common make
COMMON = .

CC = propeller-elf-gcc
OBJCOPY = propeller-elf-objcopy

# flags to use for building
CFLAGS = -Os

#
# objects for this program
#

NAME = GraphicsTest
OBJS = GraphicsTest.o Graphics.o GraphicsFont.o Graphics_firmware.o tv.o TV_firmware.o

all: $(NAME).elf

#
# include the fragment that defines most of everything
#
include $(COMMON)/common.mk
