#include <iostream>
#include "opencv2/opencv.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include "yolo_detector.hpp"

using namespace std;
using namespace cv;
using namespace cv::dnn;

static const char* params =
        "{ help           | false | print usage         }"
                "{ cfg            |       | model configuration }"
                "{ model          |       | model weights       }"
                "{ camera_device  | 0     | camera device number}"
                "{ source         |       | video or image for detection}"
                "{ min_confidence | 0.24  | min confidence      }"
                "{ class_names    |       | File with class names, [PATH-TO-DARKNET]/data/coco.names }";

int main(int argc, const char*const* argv) {

    CommandLineParser parser(argc, argv, params);
    VideoCapture cap;
    cap.open(parser.get<String>("source"));
    if(!cap.isOpened())
    {
        cout << "Couldn't open image or video: " << parser.get<String>("video") << endl;
        return -1;
    }
    for(;;)
    {
        Mat frame;
        cap >> frame;
        if (frame.empty())
        {
            waitKey();
            break;
        }
        resize(frame, frame, Size(1000, 600), 0, 0, INTER_CUBIC);
        Mat inputBlob = blobFromImage(frame, 1 / 255.F, Size(416, 416), Scalar(), true, false); //Convert Mat to batch of images
        YoloDetector yoloDetector;
        Mat detectionMat = yoloDetector.compute_output(inputBlob);
        float confidenceThreshold = 0.1;
        for (int i = 0; i < detectionMat.rows; i++)
        {
            auto scoring = yoloDetector.computeConfidence(detectionMat, i);
            float confidence = get<0>(scoring);
            size_t objectClass = get<1>(scoring);
            if (confidence > confidenceThreshold)
            {
                yoloDetector.drawFrame(detectionMat, i, frame, objectClass, confidence);
            }
        }
        imshow("YOLO: Detections", frame);
        if (waitKey(1) >= 0) break;
    }
    return 0;
}