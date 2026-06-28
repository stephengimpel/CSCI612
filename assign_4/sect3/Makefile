TARGET = captureskel
SRC = captureskel.cpp

all: captureskel contours skelcontours

captureskel:
	g++ $(SRC) -o $(TARGET) `pkg-config --cflags --libs opencv4`

contours:
	g++ contours.cpp -o contours `pkg-config --cflags --libs opencv4`

skelcontours:
	g++ skelcontours.cpp -o skelcontours `pkg-config --cflags --libs opencv4`

clean:
	rm -f $(TARGET) contours skelcontours