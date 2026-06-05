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

#define IMG_HEIGHT (240)
#define IMG_WIDTH (320)

const cv::Scalar YELLOW(0, 255, 255); // BGR

int main(int argc, char *argv[]) {

    if (argc != 3) {
        cerr << "Usage: " << argv[0]
             << " <input.ppm> <output.ppm>\n";
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];

    Mat original = imread(inputFile);

    if (original.empty()) {
        cerr << "File not found: " << argv[1];
        return 1;
    }

    Mat output = original.clone();

    // Cross hairs
    line(
        output,
        Point(0, IMG_HEIGHT / 2),
        Point(IMG_WIDTH, IMG_HEIGHT / 2),
        YELLOW,
        1
    );
    line(
        output,
        Point(IMG_WIDTH / 2, 0),
        Point(IMG_WIDTH / 2, IMG_HEIGHT),
        YELLOW,
        1
    );

    imwrite(outputFile, output);
}
