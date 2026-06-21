#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/core/core.hpp>

using namespace cv;
using namespace std;

#define WINDOW_HEIGHT (240)
#define WINDOW_WIDTH (320)

#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

int main(int argc, char **argv) {

    VideoCapture cam0(0);
    char winInput;

    if (!cam0.isOpened()) {
        exit(SYSTEM_ERROR);
    }

    cam0.set(CAP_PROP_FRAME_WIDTH, WINDOW_WIDTH);
    cam0.set(CAP_PROP_FRAME_HEIGHT, WINDOW_HEIGHT);

    // Declare the output variables
    Mat dst, cdst, cdstP, mat_gray;

    while (1) {
        if ((winInput = waitKey(1)) == ESCAPE_KEY) {
            break;
        }
        Mat src; // frame
        cam0.read(src);

        cvtColor(src, mat_gray, COLOR_BGR2GRAY);

        // Edge detection
        Canny(mat_gray, dst, 50, 200, 3);
        // Copy edges to the images that will display the results in BGR
        cvtColor(dst, cdst, COLOR_GRAY2BGR);
        cdstP = cdst.clone();
        // Standard Hough Line Transform
        vector<Vec2f> lines;                               // will hold the results of the detection
        HoughLines(dst, lines, 1, CV_PI / 180, 150, 0, 0); // runs the actual detection
        // Draw the lines
        for (size_t i = 0; i < lines.size(); i++) {
            float rho = lines[i][0], theta = lines[i][1];
            Point pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a * rho, y0 = b * rho;
            pt1.x = cvRound(x0 + 1000 * (-b));
            pt1.y = cvRound(y0 + 1000 * (a));
            pt2.x = cvRound(x0 - 1000 * (-b));
            pt2.y = cvRound(y0 - 1000 * (a));
            line(cdst, pt1, pt2, Scalar(0, 0, 255), 3, LINE_AA);
        }
        // Probabilistic Line Transform
        vector<Vec4i> linesP;                                 // will hold the results of the detection
        HoughLinesP(dst, linesP, 1, CV_PI / 180, 50, 50, 10); // runs the actual detection
        // Draw the lines
        for (size_t i = 0; i < linesP.size(); i++) {
            Vec4i l = linesP[i];
            line(cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, LINE_AA);
        }
        // Show results
        imshow("Source", src);
        imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);
        imshow("Detected Lines (in red) - Probabilistic Line Transform", cdstP);
    }
    return 0;
}