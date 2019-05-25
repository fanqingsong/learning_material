# standards
INCLUDE		= /usr/src/linux/include
CC 		= gcc
CFLAGS 		= -D__KERNEL__ -I$(INCLUDE) -DMODULE -Wall -O2

TARGET	= schar
SRC	= schar.c

all: $(TARGET).o

clean:
	rm -f *.o *~ core
