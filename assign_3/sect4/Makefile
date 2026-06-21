TARGET = capture
SRC = capture.cpp

all:
	g++ $(SRC) -o $(TARGET) `pkg-config --cflags --libs opencv4`

video:
	g++ video-capture.cpp -o video-capture `pkg-config --cflags --libs opencv4`

clean:
	rm -f $(TARGET)