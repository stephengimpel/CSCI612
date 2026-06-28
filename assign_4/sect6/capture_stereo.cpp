/*
 *
 *  Example by Sam Siewert - modified for dual USB Camera capture
 *  Updated for OpenCV 4.x C++ API
 *
 *  NOTE: Uncompressed YUV at 640x480 for 2 cameras is likely to exceed
 *        your USB 2.0 bandwidth available.  The calculation is:
 *        2 cameras x 640 x 480 x 2 bytes_per_pixel x 30 Hz = 36000 KBytes/sec
 *
 *        About 370 Mbps (assuming 8b/10b link encoding), and USB 2.0 is 480
 *        Mbps at line rate with no overhead.
 *
 *        So, for full performance, drop resolution down to 320x240.
 *
 *        I tested with really old Logitech C200 and newer C270 webcams, both are well
 *        supported and tested with the Linux UVC driver - http://www.ideasonboard.org/uvc
 *
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "opencv2/calib3d.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/videoio.hpp"

using namespace cv;
using namespace std;

// If you have enough USB 2.0 bandwidth, then run at higher resolution
// #define HRES_COLS (640)
// #define VRES_ROWS (480)

// Should always work for uncompressed USB 2.0 dual cameras
#define HRES_COLS (320)
#define VRES_ROWS (240)

#define ESC_KEY (27)

char snapshotname[80] = "snapshot_xxx.jpg";

int main(int argc, char **argv) {
    double prev_frame_time = 0, prev_frame_time_l = 0, prev_frame_time_r = 0;
    double curr_frame_time, curr_frame_time_l, curr_frame_time_r;
    struct timespec frame_time, frame_time_l, frame_time_r;
    int dev = 0, devl = 0, devr = 0;

    Mat disp;

    // StereoVar removed in OpenCV 4; replaced with SGBM which is well-supported
    Ptr<StereoSGBM> myStereo = StereoSGBM::create(
        -16, // minDisparity
        32,  // numDisparities (must be divisible by 16)
        3,   // blockSize
        72,  // P1 = 8 * channels * blockSize^2
        288, // P2 = 32 * channels * blockSize^2
        1,   // disp12MaxDiff
        63,  // preFilterCap
        10,  // uniquenessRatio
        100, // speckleWindowSize
        32,  // speckleRange
        StereoSGBM::MODE_SGBM
    );

    if (argc >= 3) {
        printf("argv[1]=%s, argv[2]=%s\n", argv[1], argv[2]);
        sscanf(argv[1], "%d", &devl);
        sscanf(argv[2], "%d", &devr);
        printf("Will open DUAL video devices %d and %d\n", devl, devr);

        VideoCapture capture_l(devl);
        VideoCapture capture_r(devr);
        capture_l.set(CAP_PROP_FRAME_WIDTH, HRES_COLS);
        capture_l.set(CAP_PROP_FRAME_HEIGHT, VRES_ROWS);
        capture_r.set(CAP_PROP_FRAME_WIDTH, HRES_COLS);
        capture_r.set(CAP_PROP_FRAME_HEIGHT, VRES_ROWS);

        if (!capture_l.isOpened() || !capture_r.isOpened()) {
            printf("Failed to open one or both cameras\n");
            return -1;
        }

        namedWindow("Capture LEFT", WINDOW_AUTOSIZE);
        namedWindow("Capture RIGHT", WINDOW_AUTOSIZE);
        namedWindow("Capture DISPARITY", WINDOW_AUTOSIZE);

        Mat frame_l, frame_r, frame_l_gray, frame_r_gray, disp8;

        while (1) {
            capture_l >> frame_l;
            capture_r >> frame_r;

            if (frame_l.empty() || frame_r.empty()) break;

            clock_gettime(CLOCK_REALTIME, &frame_time_l);
            curr_frame_time_l = ((double)frame_time_l.tv_sec * 1000.0) +
                                ((double)frame_time_l.tv_nsec / 1000000.0);

            clock_gettime(CLOCK_REALTIME, &frame_time_r);
            curr_frame_time_r = ((double)frame_time_r.tv_sec * 1000.0) +
                                ((double)frame_time_r.tv_nsec / 1000000.0);

            // SGBM works on grayscale
            cvtColor(frame_l, frame_l_gray, COLOR_BGR2GRAY);
            cvtColor(frame_r, frame_r_gray, COLOR_BGR2GRAY);

            myStereo->compute(frame_l_gray, frame_r_gray, disp);

            // Convert disparity to displayable 8-bit image
            disp.convertTo(disp8, CV_8U, 255.0 / (32 * 16.0));

            imshow("Capture LEFT", frame_l);
            imshow("Capture RIGHT", frame_r);
            imshow("Capture DISPARITY", disp8);

            printf("LEFT dt=%lf msec, RIGHT dt=%lf msec\n", (curr_frame_time_l - prev_frame_time_l), (curr_frame_time_r - prev_frame_time_r));

            char c = (char)waitKey(10);
            if (c == ESC_KEY) {
                sprintf(&snapshotname[9], "left_%8.4lf.jpg", curr_frame_time_l);
                imwrite(snapshotname, frame_l);
                sprintf(&snapshotname[9], "right_%8.4lf.jpg", curr_frame_time_r);
                imwrite(snapshotname, frame_r);
            }

            prev_frame_time_l = curr_frame_time_l;
            prev_frame_time_r = curr_frame_time_r;
        }

        capture_l.release();
        capture_r.release();
        destroyWindow("Capture LEFT");
        destroyWindow("Capture RIGHT");
        destroyWindow("Capture DISPARITY");
    } else {
        if (argc == 2) {
            printf("argv[1]=%s\n", argv[1]);
            sscanf(argv[1], "%d", &dev);
            printf("Will open SINGLE video device %d\n", dev);
        } else {
            printf("Will open DEFAULT video device video0\n");
        }

        VideoCapture capture(dev);
        capture.set(CAP_PROP_FRAME_WIDTH, HRES_COLS);
        capture.set(CAP_PROP_FRAME_HEIGHT, VRES_ROWS);

        if (!capture.isOpened()) {
            printf("Failed to open video device %d\n", dev);
            return -1;
        }

        namedWindow("Capture Example", WINDOW_AUTOSIZE);

        Mat frame;

        while (1) {
            capture >> frame;

            if (frame.empty()) break;

            clock_gettime(CLOCK_REALTIME, &frame_time);
            curr_frame_time = ((double)frame_time.tv_sec * 1000.0) +
                              ((double)frame_time.tv_nsec / 1000000.0);

            imshow("Capture Example", frame);
            printf("Frame time = %u sec, %lu nsec, dt=%lf msec\n", (unsigned)frame_time.tv_sec, (unsigned long)frame_time.tv_nsec, (curr_frame_time - prev_frame_time));

            char c = (char)waitKey(10);
            if (c == ESC_KEY) {
                sprintf(&snapshotname[9], "%8.4lf.jpg", curr_frame_time);
                imwrite(snapshotname, frame);
            }

            prev_frame_time = curr_frame_time;
        }

        capture.release();
        destroyWindow("Capture Example");
    }

    return 0;
}