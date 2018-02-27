#include <iostream>
#include "opencv2/opencv.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>

using namespace std;
using namespace cv;
using namespace cv::dnn;

class YoloDetector {
    String MODEL_CONFIG = "/home/vagrant/darknet/cfg/yolo.cfg";
    String MODEL_BINARY = "/home/vagrant/darknet/yolo.weights";
    dnn::Net net;

public:
    YoloDetector();
    Mat compute_output(Mat input);

};

YoloDetector::YoloDetector() {
    net = readNetFromDarknet(MODEL_CONFIG, MODEL_BINARY);
}

Mat YoloDetector::compute_output(Mat input) {
    net.setInput(input, "data");                   //set the network input
    Mat detectionMat = net.forward("detection_out");   //compute output
    return detectionMat;
}