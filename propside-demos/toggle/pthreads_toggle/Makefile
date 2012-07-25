# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../../common

CC = propeller-elf-gcc

CFLAGS = -Os -mlmm 
#-Dprintf=__simple_printf
LIBS = -lpthread

#
# objects for this program
#

NAME = toggle
OBJS = toggle.o

all: $(NAME).elf

$(NAME).elf: $(OBJS)
	$(CC) -Os -o $@ $^ -s -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.elf

run: all
	propeller-load -r -t $(NAME).elf
