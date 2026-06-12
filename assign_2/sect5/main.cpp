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

enum states {
    NONE,
    SOBEL,
    CANNY
};

int lowThreshold = 0;
const int max_lowThreshold = 100;
const int canny_ratio = 3;
const int kernel_size = 3;
const int scale = 1;
const int delta = 0;

int main(int argc, char *argv[]) {
    VideoCapture cam0(0);
    namedWindow("video_display");
    char winInput;

    if (!cam0.isOpened()) {
        exit(SYSTEM_ERROR);
    }

    cam0.set(CAP_PROP_FRAME_WIDTH, WINDOW_WIDTH);
    cam0.set(CAP_PROP_FRAME_HEIGHT, WINDOW_HEIGHT);
    Mat mat_gray, dst, canny_edges, grad_x, grad_y, abs_grad_x, abs_grad_y;
    states current_state = NONE;
    while (1) {
        if ((winInput = waitKey(1)) == ESCAPE_KEY) {
            break;
        } else if (winInput == 'n') {
            cout << "None selected" << endl;
            current_state = NONE;
        } else if (winInput == 's') {
            cout << "Sobel selected" << endl;
            current_state = SOBEL;
        } else if (winInput == 'c') {
            cout << "Canny selected" << endl;
            current_state = CANNY;
        }
        Mat frame;
        cam0.read(frame);
        cvtColor(frame, mat_gray, COLOR_BGR2GRAY);

        if (current_state == CANNY) {
            dst.create(frame.size(), frame.type());
            blur(mat_gray, canny_edges, Size(3, 3)); // should blur match kerenal size?
            Canny(canny_edges, canny_edges, lowThreshold, lowThreshold * canny_ratio, kernel_size);
            dst.setTo(Scalar::all(0));
            frame.copyTo(dst, canny_edges);
            frame = dst;
        } else if (current_state == SOBEL) {
            GaussianBlur(mat_gray, frame, Size(3, 3), 0, 0, BORDER_DEFAULT);
            Sobel(mat_gray, grad_x, CV_16S, 1, 0, kernel_size, scale, delta, BORDER_DEFAULT);
            Sobel(mat_gray, grad_y, CV_16S, 0, 1, kernel_size, scale, delta, BORDER_DEFAULT);
            convertScaleAbs(grad_x, abs_grad_x);
            convertScaleAbs(grad_y, abs_grad_y);
            addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);
            frame = dst;
        }

        imshow("video_display", frame);
    }

    destroyWindow("video_display");
}
