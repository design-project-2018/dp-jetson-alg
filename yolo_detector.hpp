#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

/**
 * Interface for the YOLO Detector Class which processes object classification using the YOLO technique.
 */
class YoloDetector {

public:
    YoloDetector();
    Mat compute_output(Mat input);
    pair<float, size_t> computeConfidence(Mat detectionMat, int index);
    void drawFrame(Mat detectionMat, int index, Mat &frame, size_t objectClass, float confidence);
};
