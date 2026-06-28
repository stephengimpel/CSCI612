/*
 *  stereo_match.cpp
 *  Updated for OpenCV 4.x
 *  Original by Victor Eruhimov on 1/18/10
 */

#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include <stdio.h>

using namespace cv;

void print_help() {
    printf("\nDemo stereo matching converting L and R images into disparity and point clouds\n");
    printf("\nUsage: stereo_match <left_image> <right_image> [--algorithm=bm|sgbm|hh] [--blocksize=<block_size>]\n"
           "[--max-disparity=<max_disparity>] [--scale=<scale_factor>] [-i <intrinsic_filename>] [-e <extrinsic_filename>]\n"
           "[--no-display] [-o <disparity_image>] [-p <point_cloud_file>]\n");
}

void saveXYZ(const char *filename, const Mat &mat) {
    const double max_z = 1.0e4;
    FILE *fp = fopen(filename, "wt");
    for (int y = 0; y < mat.rows; y++) {
        for (int x = 0; x < mat.cols; x++) {
            Vec3f point = mat.at<Vec3f>(y, x);
            if (fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z) continue;
            fprintf(fp, "%f %f %f\n", point[0], point[1], point[2]);
        }
    }
    fclose(fp);
}

int main(int argc, char **argv) {
    const char *algorithm_opt = "--algorithm=";
    const char *maxdisp_opt = "--max-disparity=";
    const char *blocksize_opt = "--blocksize=";
    const char *nodisplay_opt = "--no-display";
    const char *scale_opt = "--scale=";

    if (argc < 3) {
        print_help();
        return 0;
    }

    const char *img1_filename = 0;
    const char *img2_filename = 0;
    const char *intrinsic_filename = 0;
    const char *extrinsic_filename = 0;
    const char *disparity_filename = 0;
    const char *point_cloud_filename = 0;

    // StereoVar removed in OpenCV 4; only BM, SGBM, and HH (SGBM full DP) remain
    enum {
        STEREO_BM = 0,
        STEREO_SGBM = 1,
        STEREO_HH = 2
    };
    int alg = STEREO_SGBM;
    int SADWindowSize = 0, numberOfDisparities = 0;
    bool no_display = false;
    float scale = 1.f;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (!img1_filename)
                img1_filename = argv[i];
            else
                img2_filename = argv[i];
        } else if (strncmp(argv[i], algorithm_opt, strlen(algorithm_opt)) == 0) {
            char *_alg = argv[i] + strlen(algorithm_opt);
            alg = strcmp(_alg, "bm") == 0 ? STEREO_BM : strcmp(_alg, "sgbm") == 0 ? STEREO_SGBM
                                                    : strcmp(_alg, "hh") == 0     ? STEREO_HH
                                                                                  : -1;
            if (alg < 0) {
                printf("Command-line parameter error: Unknown stereo algorithm\n\n");
                print_help();
                return -1;
            }
        } else if (strncmp(argv[i], maxdisp_opt, strlen(maxdisp_opt)) == 0) {
            if (sscanf(argv[i] + strlen(maxdisp_opt), "%d", &numberOfDisparities) != 1 ||
                numberOfDisparities < 1 || numberOfDisparities % 16 != 0) {
                printf("Command-line parameter error: --max-disparity must be a positive integer divisible by 16\n");
                return -1;
            }
        } else if (strncmp(argv[i], blocksize_opt, strlen(blocksize_opt)) == 0) {
            if (sscanf(argv[i] + strlen(blocksize_opt), "%d", &SADWindowSize) != 1 ||
                SADWindowSize < 1 || SADWindowSize % 2 != 1) {
                printf("Command-line parameter error: --blocksize must be a positive odd number\n");
                return -1;
            }
        } else if (strncmp(argv[i], scale_opt, strlen(scale_opt)) == 0) {
            if (sscanf(argv[i] + strlen(scale_opt), "%f", &scale) != 1 || scale < 0) {
                printf("Command-line parameter error: --scale must be a positive float\n");
                return -1;
            }
        } else if (strcmp(argv[i], nodisplay_opt) == 0)
            no_display = true;
        else if (strcmp(argv[i], "-i") == 0)
            intrinsic_filename = argv[++i];
        else if (strcmp(argv[i], "-e") == 0)
            extrinsic_filename = argv[++i];
        else if (strcmp(argv[i], "-o") == 0)
            disparity_filename = argv[++i];
        else if (strcmp(argv[i], "-p") == 0)
            point_cloud_filename = argv[++i];
        else {
            printf("Command-line parameter error: unknown option %s\n", argv[i]);
            return -1;
        }
    }

    if (!img1_filename || !img2_filename) {
        printf("Command-line parameter error: both left and right images must be specified\n");
        return -1;
    }

    if ((intrinsic_filename != 0) ^ (extrinsic_filename != 0)) {
        printf("Command-line parameter error: intrinsic and extrinsic parameters must both be specified, or neither\n");
        return -1;
    }

    if (extrinsic_filename == 0 && point_cloud_filename) {
        printf("Command-line parameter error: intrinsic/extrinsic parameters required for point cloud\n");
        return -1;
    }

    int color_mode = alg == STEREO_BM ? 0 : -1;
    Mat img1 = imread(img1_filename, color_mode);
    Mat img2 = imread(img2_filename, color_mode);

    if (img1.empty() || img2.empty()) {
        printf("Error: could not load images\n");
        return -1;
    }

    if (scale != 1.f) {
        Mat temp1, temp2;
        int method = scale < 1 ? INTER_AREA : INTER_CUBIC;
        resize(img1, temp1, Size(), scale, scale, method);
        img1 = temp1;
        resize(img2, temp2, Size(), scale, scale, method);
        img2 = temp2;
    }

    Size img_size = img1.size();
    Rect roi1, roi2;
    Mat Q;

    if (intrinsic_filename) {
        FileStorage fs(intrinsic_filename, FileStorage::READ);
        if (!fs.isOpened()) {
            printf("Failed to open file %s\n", intrinsic_filename);
            return -1;
        }

        Mat M1, D1, M2, D2;
        fs["M1"] >> M1;
        fs["D1"] >> D1;
        fs["M2"] >> M2;
        fs["D2"] >> D2;

        fs.open(extrinsic_filename, FileStorage::READ);
        if (!fs.isOpened()) {
            printf("Failed to open file %s\n", extrinsic_filename);
            return -1;
        }

        Mat R, T, R1, P1, R2, P2;
        fs["R"] >> R;
        fs["T"] >> T;

        stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, img_size, &roi1, &roi2);

        Mat map11, map12, map21, map22;
        initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
        initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);

        Mat img1r, img2r;
        remap(img1, img1r, map11, map12, INTER_LINEAR);
        remap(img2, img2r, map21, map22, INTER_LINEAR);

        img1 = img1r;
        img2 = img2r;
    }

    numberOfDisparities = numberOfDisparities > 0 ? numberOfDisparities : ((img_size.width / 8) + 15) & -16;

    // Modern Ptr-based API for BM and SGBM
    Ptr<StereoBM> bm = StereoBM::create(numberOfDisparities, SADWindowSize > 0 ? SADWindowSize : 9);
    bm->setROI1(roi1);
    bm->setROI2(roi2);
    bm->setPreFilterCap(31);
    bm->setMinDisparity(0);
    bm->setTextureThreshold(10);
    bm->setUniquenessRatio(15);
    bm->setSpeckleWindowSize(100);
    bm->setSpeckleRange(32);
    bm->setDisp12MaxDiff(1);

    int cn = img1.channels();
    int sad = SADWindowSize > 0 ? SADWindowSize : 3;
    Ptr<StereoSGBM> sgbm = StereoSGBM::create(
        0,                   // minDisparity
        numberOfDisparities, // numDisparities
        sad,                 // blockSize
        8 * cn * sad * sad,  // P1
        32 * cn * sad * sad, // P2
        1,                   // disp12MaxDiff
        63,                  // preFilterCap
        10,                  // uniquenessRatio
        100,                 // speckleWindowSize
        32,                  // speckleRange
        alg == STEREO_HH ? StereoSGBM::MODE_HH : StereoSGBM::MODE_SGBM
    );

    Mat disp, disp8;

    int64 t = getTickCount();
    if (alg == STEREO_BM)
        bm->compute(img1, img2, disp);
    else if (alg == STEREO_SGBM || alg == STEREO_HH)
        sgbm->compute(img1, img2, disp);
    t = getTickCount() - t;
    printf("Time elapsed: %fms\n", t * 1000 / getTickFrequency());

    disp.convertTo(disp8, CV_8U, 255.0 / (numberOfDisparities * 16.0));

    if (!no_display) {
        namedWindow("left", 1);
        imshow("left", img1);
        namedWindow("right", 1);
        imshow("right", img2);
        namedWindow("disparity", 0);
        imshow("disparity", disp8);
        printf("press any key to continue...");
        fflush(stdout);
        waitKey();
        printf("\n");
    }

    if (disparity_filename)
        imwrite(disparity_filename, disp8);

    if (point_cloud_filename) {
        printf("storing the point cloud...");
        fflush(stdout);
        Mat xyz;
        reprojectImageTo3D(disp, xyz, Q, true);
        saveXYZ(point_cloud_filename, xyz);
        printf("\n");
    }

    return 0;
}