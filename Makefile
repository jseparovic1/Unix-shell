CC = gcc
CFLAGS = -Wall

TARGETS = jusepShell

default: jusepShell

all: $(TARGETS)

jusepShell: jusepShell.o
	$(CC) $(CFLAGS) jusepShell.o -o jusepShell

clean:
	rm -f *.o *~ a.out $(TARGETS)

.c.o:
	$(CC) $(CFLAGS) -c $<
