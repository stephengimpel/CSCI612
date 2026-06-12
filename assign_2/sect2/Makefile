TARGET = sobel
SRC = sobel.cpp

all:
	g++ $(SRC) -o $(TARGET) `pkg-config --cflags --libs opencv4`

clean:
	rm -f $(TARGET)