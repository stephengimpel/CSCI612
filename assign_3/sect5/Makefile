TARGET = grayscale
SRC = grayscale.cpp

all:
	g++ $(SRC) -o $(TARGET) `pkg-config --cflags --libs opencv4`

red:
	g++ redscale.cpp -o redscale `pkg-config --cflags --libs opencv4`

clean:
	rm -f $(TARGET)