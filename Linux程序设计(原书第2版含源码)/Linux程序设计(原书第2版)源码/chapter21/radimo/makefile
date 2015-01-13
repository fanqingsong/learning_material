# standards
INCLUDE		= /usr/src/linux/include
CC 		= gcc
CFLAGS 		= -D__KERNEL__ -I$(INCLUDE) -DMODULE -Wall -O2

TARGET	= radimo
SRC	= radimo.c	

all: $(TARGET).o

clean:
	rm -f *.o *~ core
