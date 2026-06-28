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

/* Notes on hyper params, all variable names are the same as the example code from open CV
  rho - resolution of parameter "r" in pixels. smaller => percision, larger => merge nearby lines
  theta - angle resolution in radians. Smaller => precision but more computation
  threshold - min # of "votes" required for line detection. Confidence metric
  srn - distance resolution used by multi-scale hough transforms
  stn - angular resoltuion used for multi-scale transforms
*/
struct HoughParams {
    double rho = 1.0;           // OpenCV Default 1
    double theta = CV_PI / 180; // OpenCV Default CV_PI/180 or "1 degree"
    int threshold = 165;        // OpenCV Default 150
    double srn = 0.0;           // OpenCV default 0.0
    double stn = 0.0;           // OpenCV Default 0.0
};

/*
    rho - distance resolution, same as above
    theta - angular resolution, same as above
    threshold - confidence, same as above
    minLineLength - threshold to ignore detected lines shorter than X pixels
    maxLineGap - maximum gap between two different segments that can be merged into a single line
*/
struct HoughParamsP {
    double rho = 1.0;           // OpenCV Default 1
    double theta = CV_PI / 180; // OpenCV Default CV_PI/180 or "1 degree"
    int threshold = 90;         // OpenCV Default 50
    double minLineLength = 75;  // OpenCV Default 50
    double maxLineGap = 15;     // OpenCV Default 10
};

int main(int argc, char **argv) {

    HoughParams hough;
    HoughParamsP houghP;

    bool useFile = argc >= 2;

    VideoCapture videoInput;
    if (!useFile) {
        videoInput.open(0);

        if (!videoInput.isOpened()) {
            exit(SYSTEM_ERROR);
        }

        videoInput.set(CAP_PROP_FRAME_WIDTH, WINDOW_WIDTH);
        videoInput.set(CAP_PROP_FRAME_HEIGHT, WINDOW_HEIGHT);
    } else {
        videoInput.open(argv[1]);
    }
    char winInput;

    // Declare the output variables
    Mat dst, cdst, cdstP, mat_gray;

    while (1) {
        if ((winInput = waitKey(1)) == ESCAPE_KEY) {
            break;
        }
        Mat src; // frame

        videoInput.read(src);

        if (src.empty()) {
            break;
        }

        cvtColor(src, mat_gray, COLOR_BGR2GRAY);

        // Edge detection
        Canny(mat_gray, dst, 50, 200, 3);
        // Copy edges to the images that will display the results in BGR
        cvtColor(dst, cdst, COLOR_GRAY2BGR);
        cdstP = cdst.clone();
        // Standard Hough Line Transform
        vector<Vec2f> lines; // will hold the results of the detection
        HoughLines(
            dst,
            lines,
            hough.rho,
            hough.theta,
            hough.threshold,
            hough.srn,
            hough.stn
        ); // runs the actual detection
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
        vector<Vec4i> linesP; // will hold the results of the detection
        HoughLinesP(
            dst,
            linesP,
            houghP.rho,
            houghP.theta,
            houghP.threshold,
            houghP.minLineLength,
            houghP.maxLineGap
        ); // runs the actual detection
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