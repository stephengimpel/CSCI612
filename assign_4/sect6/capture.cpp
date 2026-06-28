/*
 *
 *  Example by Sam Siewert
 *  Updated for OpenCV 4.x C++ API
 *
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    int dev = 0;

    if (argc > 1) {
        printf("argv[1]=%s\n", argv[1]);
        sscanf(argv[1], "%d", &dev);
        printf("Will open video device %d\n", dev);
    }

    VideoCapture capture(dev);

    if (!capture.isOpened()) {
        printf("Failed to open video device %d\n", dev);
        return -1;
    }

    namedWindow("Capture Example", WINDOW_AUTOSIZE);

    Mat frame;

    while (1) {
        capture >> frame;

        if (frame.empty()) break;

        imshow("Capture Example", frame);

        char c = (char)waitKey(33);
        if (c == 27) break;
    }

    capture.release();
    destroyWindow("Capture Example");

    return 0;
}