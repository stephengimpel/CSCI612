#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

char difftext[20];

int main(int argc, char **argv) {
    Mat frame, red;
    VideoCapture vcap;
    vector<Mat> channels;

    const char *default_file = "../Dark-Room-Laser-Spot.mpeg";
    const char *filename = argc >= 2 ? argv[1] : default_file;
    if (!vcap.open(filename)) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    cout << "video opened" << endl;

    int width = (int)vcap.get(CAP_PROP_FRAME_WIDTH);
    int height = (int)vcap.get(CAP_PROP_FRAME_HEIGHT);
    double fps = vcap.get(CAP_PROP_FPS);

    if (fps <= 0) fps = 30.0;

    VideoWriter writer(
        "output_red.mp4",
        VideoWriter::fourcc('m', 'p', '4', 'v'),
        fps,
        Size(width, height),
        false
    );
    if (!writer.isOpened()) {
        cout << "Could not open output video writer" << endl;
        return -1;
    }

    while (true) {

        if (!vcap.read(frame)) break;

        split(frame, channels);

        red = channels[2];

        writer.write(red);

        imshow("red Frame", red);

        char c = (char)waitKey(1);
        if (c == 'q') break;
    }

    vcap.release();
    writer.release();
    destroyAllWindows();

    return 0;
};
