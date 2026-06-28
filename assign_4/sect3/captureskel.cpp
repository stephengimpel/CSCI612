/*
 *
 *  Example by Sam Siewert
 *
 *  Created for OpenCV 4.x for Jetson Nano 2g, based upon
 *  https://docs.opencv.org/4.1.1
 *
 *  Tested with JetPack 4.6 which installs OpenCV 4.1.1
 *  (https://developer.nvidia.com/embedded/jetpack)
 *
 *  Based upon earlier simpler-capture examples created
 *  for OpenCV 2.x and 3.x (C and C++ mixed API) which show
 *  how to use OpenCV instead of lower level V4L2 API for the
 *  Linux UVC driver.
 *
 *  Verify your hardware and OS configuration with:
 *  1) lsusb
 *  2) ls -l /dev/video*
 *  3) dmesg | grep UVC
 *
 *  Note that OpenCV 4.x only supports the C++ API
 *
 *
 *  FOR REFRENCE, NOT IN THE MAKE FILE
 */
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

// See www.asciitable.com
#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

int main() {
    VideoCapture cam0(0);
    char winInput;

    if (!cam0.isOpened()) {
        exit(SYSTEM_ERROR);
    }

    cam0.set(CAP_PROP_FRAME_WIDTH, 640);
    cam0.set(CAP_PROP_FRAME_HEIGHT, 480);

    Mat gray, binary, mfblur;

    int framecount = 0;

    while (1) {
        Mat src;

        cam0.read(src);
        cvtColor(src, gray, COLOR_BGR2GRAY);

        threshold(gray, binary, 150, 255, THRESH_BINARY);
        binary = 255 - binary;

        medianBlur(binary, mfblur, 1);

        Mat skel(mfblur.size(), CV_8UC1, Scalar(0));

        Mat temp;
        Mat eroded;
        Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));

        bool done;
        int iterations = 0;

        do {
            erode(mfblur, eroded, element);
            dilate(eroded, temp, element);
            subtract(mfblur, temp, temp);
            bitwise_or(skel, temp, skel);
            eroded.copyTo(mfblur);

            done = (countNonZero(mfblur) == 0);
            iterations++;
        } while (!done && (iterations < 300));

        cout << "iterations=" << iterations << endl;

        // imshow("src", src);
        // imshow("graymap", gray);
        // imshow("binary", binary);
        imshow("skeleton", skel);

        if ((winInput = waitKey(1)) == ESCAPE_KEY)
        // if ((winInput = waitKey(0)) == ESCAPE_KEY)
        {
            break;
        } else if (winInput == 'n') {
            cout << "input " << winInput << " ignored" << endl;
        }
        ostringstream filename;
        filename << "output/skel_" << setw(6) << setfill('0') << framecount++ << +".png";

        imwrite(filename.str(), skel);
        // breaks after 3000 frames recorded
        if (framecount > 3000) {
            break;
        }
    }
};
