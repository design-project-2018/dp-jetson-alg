#include <iostream>
#include "opencv2/opencv.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include "yolo_detector.hpp"

using namespace std;
using namespace cv;
using namespace cv::dnn;

String MODEL_CONFIG = "/home/nvidia/Desktop/dp-jetson-alg/darknet/cfg/yolo.cfg";
String MODEL_BINARY = "/home/nvidia/Desktop/dp-jetson-alg/darknet/yolo.weights";
String CLASS_NAMES = "/home/nvidia/Desktop/dp-jetson-alg/darknet/data/coco.names";

cv::dnn::Net net;
vector<string> classNamesVec;

/**
 * Constructor method for the YOLO Detector Class
 *
 * @return
 */
YoloDetector::YoloDetector() {
    net = readNetFromDarknet(MODEL_CONFIG, MODEL_BINARY);
    ifstream classNamesFile(CLASS_NAMES.c_str());
    string className = "";
    while (std::getline(classNamesFile, className))
        classNamesVec.push_back(className);
}

/**
 * Computes the output of the YOLO detection
 *
 * @param input
 * @return
 */
Mat YoloDetector::compute_output(Mat input) {
    net.setInput(input, "data");                   //set the network input
    Mat detectionMat = net.forward("detection_out");   //compute output
    return detectionMat;
}

/**
 * Computes the confidence score and the class of a detected object
 *
 * @param detectionMat
 * @param index
 * @return
 */
pair<float, size_t> YoloDetector::computeConfidence(Mat detectionMat, int index) {
    const int probability_index = 5;
    const int probability_size = detectionMat.cols - probability_index;
    float *prob_array_ptr = &detectionMat.at<float>(index, probability_index);
    size_t objectClass = max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr;
    float confidence = detectionMat.at<float>(index, (int)objectClass + probability_index);
    return make_pair(confidence, objectClass);
}

/**
 * Draws the frame around a detected object
 *
 * @param detectionMat
 * @param index
 * @param frame
 * @param objectClass
 * @param confidence
 */
void YoloDetector::drawFrame(Mat detectionMat, int index, Mat &frame, size_t objectClass, float confidence) {
    float x = detectionMat.at<float>(index, 0);
    float y = detectionMat.at<float>(index, 1);
    float width = detectionMat.at<float>(index, 2);
    float height = detectionMat.at<float>(index, 3);
    int xLeftBottom = static_cast<int>((x - width / 2) * frame.cols);
    int yLeftBottom = static_cast<int>((y - height / 2) * frame.rows);
    int xRightTop = static_cast<int>((x + width / 2) * frame.cols);
    int yRightTop = static_cast<int>((y + height / 2) * frame.rows);
    Rect object(xLeftBottom, yLeftBottom,
                xRightTop - xLeftBottom,
                yRightTop - yLeftBottom);
    rectangle(frame, object, Scalar(0, 255, 0));
    if (objectClass < classNamesVec.size())
    {
        ostringstream ss;
        ss << confidence;
        String conf(ss.str());
        String label = String(classNamesVec[objectClass]) + ": " + conf;
        int baseLine = 0;
        Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        rectangle(frame, Rect(Point(xLeftBottom, yLeftBottom ),
                              Size(labelSize.width, labelSize.height + baseLine)),
                  Scalar(255, 255, 255), CV_FILLED);
        putText(frame, label, Point(xLeftBottom, yLeftBottom+labelSize.height),
                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
    }
    return;
}
