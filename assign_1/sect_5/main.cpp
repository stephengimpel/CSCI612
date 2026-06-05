#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define WINDOW_HEIGHT (240)
#define WINDOW_WIDTH (320)

#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

const cv::Scalar YELLOW(0, 255, 255); // BGR

int main(int argc, char *argv[]) {

    VideoCapture cam0(0);
    namedWindow("video_display");
    char winInput;

    if (!cam0.isOpened()) {
        exit(SYSTEM_ERROR);
    }

    cam0.set(CAP_PROP_FRAME_WIDTH, WINDOW_WIDTH);
    cam0.set(CAP_PROP_FRAME_HEIGHT, WINDOW_HEIGHT);

    while (1) {
        Mat frame;

        cam0.read(frame);
        // Cross hairs
        line(
            frame,
            Point(0, WINDOW_HEIGHT / 2),
            Point(WINDOW_WIDTH, WINDOW_HEIGHT / 2),
            YELLOW,
            1
        );
        line(
            frame,
            Point(WINDOW_WIDTH / 2, 0),
            Point(WINDOW_WIDTH / 2, WINDOW_HEIGHT),
            YELLOW,
            1
        );
        imshow("video_display", frame);

        if ((winInput = waitKey(1)) == ESCAPE_KEY) {
            break;
        } else if (winInput == 'n') {
            cout << "input " << winInput << " ignored" << endl;
        }
    }

    destroyWindow("video_display");
}
