#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
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
    Mat gray;
    while (1) {
        if ((winInput = waitKey(1)) == ESCAPE_KEY) {
            break;
        }
        Mat src; // frame
        cam0.read(src);
        cvtColor(src, gray, COLOR_BGR2GRAY);
        medianBlur(gray, gray, 5);
        vector<Vec3f> circles;
        HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                     gray.rows / 16, // change this value to detect circles with different distances to each other
                     100, 30, 1, 30  // change the last two parameters
                                     // (min_radius & max_radius) to detect larger circles
        );
        for (size_t i = 0; i < circles.size(); i++) {
            Vec3i c = circles[i];
            Point center = Point(c[0], c[1]);
            // circle center
            circle(src, center, 1, Scalar(0, 100, 100), 3, LINE_AA);
            // circle outline
            int radius = c[2];
            circle(src, center, radius, Scalar(255, 0, 255), 3, LINE_AA);
        }
        imshow("detected circles", src);
    };
    return 0;
}