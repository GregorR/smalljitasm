CC=gcc
ECFLAGS=-g
CFLAGS=$(ECFLAGS)
AR=ar
ARFLAGS=rc
RANLIB=ranlib

OBJS=arch.o arch-instructions.o

all: libsmalljitasm.a

libsmalljitasm.a: $(OBJS)
	$(AR) $(ARFLAGS) libsmalljitasm.a $(OBJS)
	$(RANLIB) libsmalljitasm.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) libsmalljitasm.a deps

include deps

deps:
	-$(CC) -MM *.c > deps
