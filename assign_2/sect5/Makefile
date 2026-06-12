TARGET = main
SRC = main.cpp

all:
	g++ $(SRC) -o $(TARGET) `pkg-config --cflags --libs opencv4`

image:
	g++ image_crosshair.cpp -o image_crosshair `pkg-config --cflags --libs opencv4`

clean:
	rm -f $(TARGET)