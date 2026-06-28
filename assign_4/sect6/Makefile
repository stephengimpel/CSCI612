INCLUDE_DIRS =
LIB_DIRS =
CC=g++

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt
CPPLIBS= -L/usr/local/opencv/lib -lopencv_core -lopencv_flann -lopencv_video
OPENCV_FLAGS= $(shell pkg-config --cflags --libs opencv4)

HFILES=
CFILES=

SRCS= ${HFILES} ${CFILES}

all:	capture stereo_match capture_stereo

clean:
	-rm -f *.o *.d
	-rm -f capture
	-rm -f capture_stereo
	-rm -f stereo_match

distclean:
	-rm -f *.o *.d

capture_stereo: capture_stereo.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(OPENCV_FLAGS) $(CPPLIBS)

capture: capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(OPENCV_FLAGS) $(CPPLIBS)

stereo_match: stereo_match.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(OPENCV_FLAGS) $(CPPLIBS)

depend:

%.o: %.c
	$(CC) $(CFLAGS) $(OPENCV_FLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(OPENCV_FLAGS) -c $< -o $@