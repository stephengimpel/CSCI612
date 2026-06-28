INCLUDE_DIRS = 
LIB_DIRS = 
CC=g++

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt
CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video
OPENCV_FLAGS= $(shell pkg-config --cflags --libs opencv4)

HFILES= 
CFILES= 
CPPFILES= sift.cpp descriptor_extractor_matcher.cpp

SRCS= ${HFILES} ${CFILES}
CPPOBJS= ${CPPFILES:.cpp=.o}

all:	sift descriptor_extractor_matcher

clean:
	-rm -f *.o *.d cvtest*.ppm cvtest*.pgm test*.ppm test*.pgm
	-rm -f sift descriptor_extractor_matcher

distclean:
	-rm -f *.o *.d

descriptor_extractor_matcher: descriptor_extractor_matcher.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(OPENCV_FLAGS) $(CPPLIBS)

sift: sift.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(OPENCV_FLAGS) $(CPPLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(OPENCV_FLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(OPENCV_FLAGS) -c $< -o $@