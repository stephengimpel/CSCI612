INCLUDE_DIRS =
LIB_DIRS =
CC = gcc

CDEFS =
CFLAGS = -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS = -lrt

HFILES =
CFILES = capture.c simple-in-out.c diff-capture.c clutter-capture.c

SRCS = $(HFILES) $(CFILES)
OBJS = $(CFILES:.c=.o)

all: capture io diff-capture clutter-capture

clean:
	-rm -f *.o *.d
	-rm -f capture
	-rm -f io
	-rm -f clutter-capture
	-rm -f diff-capture

distclean:
	-rm -f *.o *.d

capture: capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

io: simple-in-out.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

diff-capture: diff-capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

clutter-capture: clutter-capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<