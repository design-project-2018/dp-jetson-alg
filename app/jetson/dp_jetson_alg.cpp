#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <vector>
#include <stdio.h>
#include <iostream>

#include <yolo.h>

using namespace cv;
using namespace std;

int FRAME_FREQ = 4;

int main(int argc, char** argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: %s <videofile>\n", argv[0]);
        return 0;
    }

    // capture video
    VideoCapture capture(argv[1]);
    if(!capture.isOpened())
    {
        cout << "cannot read video file" << std::endl;
        return 0;
    }

    // to store current frame
    Mat frame;

    // instantiate yolo functions
    Yolo yolo;
    yolo.setConfigFilePath("/home/nvidia/Desktop/darknet/cfg/yolov2.cfg");
    yolo.setDataFilePath("/home/nvidia/Desktop/darknet/cfg/coco.data");
    yolo.setWeightFilePath("/home/nvidia/Desktop/darknet/yolov2.weights");
    yolo.setNameListFile("/home/nvidia/Desktop/darknet/data/coco.names");
    yolo.setThreshold(0.5);

    // instantiate farneback objects
    Mat flow;
    UMat flowUmat, prevgray;

    // repeatedly do computations for each frame
    while(true) {

        // capture frame
        for (int i=0; i<FRAME_FREQ; i++) {
            capture >> frame;
            if(frame.empty())
                break;
        }

        // resize frame
        resize(frame, frame, Size(600,400));

        // frames
        Mat originalFrame;
        Mat yoloFrame;
        Mat flowFrame;
        frame.copyTo(originalFrame);
        frame.copyTo(yoloFrame);
        frame.copyTo(flowFrame);

        // make copy of frame and turn it into gray for optical flow computation
        Mat img;
        frame.copyTo(img);
        cvtColor(img, img, COLOR_BGR2GRAY);

        // if a previous frame exists go ahead with computations
        if (prevgray.empty() == false ) {

            // perform optical flow
            calcOpticalFlowFarneback(prevgray, img, flowUmat, 0.4, 1, 12, 2, 8, 1.2, 0);
            flowUmat.copyTo(flow);

            // perform yolo
            vector<DetectedObject> detection;
            yolo.detect(frame, detection);

            // draw results
            for(int i = 0; i < detection.size(); i++) {
                DetectedObject &o = detection[i];

                // draw object boxes
                rectangle(frame, o.bounding_box, Scalar(0, 0, 255), 2);
                rectangle(yoloFrame, o.bounding_box, Scalar(0, 0, 255), 2);
                const char *class_name = yolo.getNames()[o.object_class];
                char str[255];
                sprintf(str, "%s", class_name);
                putText(frame, str, Point2f(o.bounding_box.x, o.bounding_box.y), FONT_HERSHEY_SIMPLEX, 0.6,
                        Scalar(0, 0, 255), 2);
                putText(yoloFrame, str, Point2f(o.bounding_box.x, o.bounding_box.y), FONT_HERSHEY_SIMPLEX, 0.6,
                        Scalar(0, 0, 255), 2);

                // draw flow vectors
                for (int y = o.bounding_box.y; y < o.bounding_box.y + o.bounding_box.height; y += 12) {
                    for (int x = o.bounding_box.x; x < o.bounding_box.x + o.bounding_box.width; x += 12) {
                        // get the flow from y, x position * 10 for better visibility
                        const Point2f flowatxy = flow.at<Point2f>(y, x) * 10;
                        // draw line at flow direction
                        line(frame, Point(x, y), Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)),
                             Scalar(255, 0, 0));
                        line(flowFrame, Point(x, y), Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)),
                             Scalar(255, 0, 0));
                        // draw initial point
                        circle(frame, Point(x, y), 1, Scalar(0, 0, 0), -1);
                        circle(flowFrame, Point(x, y), 1, Scalar(0, 0, 0), -1);
                    }
                }

            }
            // display
            imshow("Original", originalFrame);
            imshow("YOLO", yoloFrame);
            imshow("Farneback", flowFrame);
            imshow("Combined", frame);
            waitKey(1);
        } else {
            img.copyTo(prevgray);
        }
    }
}
