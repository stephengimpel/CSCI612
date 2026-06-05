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

    // // Top Line
    line(
        output,
        Point(0, 0),
        Point(IMG_WIDTH, 0),
        YELLOW,
        3
    );

    // Bottom Line
    line(
        output,
        Point(0, IMG_HEIGHT - 1),
        Point(IMG_WIDTH, IMG_HEIGHT - 1),
        YELLOW,
        3
    );

    // // Left Line
    line(
        output,
        Point(0, 0),
        Point(0, IMG_HEIGHT),
        YELLOW,
        3
    );

    // Right Line
    line(
        output,
        Point(IMG_WIDTH - 1, 0),
        Point(IMG_WIDTH - 1, IMG_HEIGHT),
        YELLOW,
        3
    );

    // Diagonals
    line(
        output,
        Point(0, 0),
        Point(IMG_WIDTH, IMG_HEIGHT),
        YELLOW,
        3
    );
    line(
        output,
        Point(IMG_WIDTH, 0),
        Point(0, IMG_HEIGHT),
        YELLOW,
        3
    );

    imwrite(outputFile, output);
}
