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

int main(int argc, char** argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: %s <videofile>\n", argv[0]);
        return 0;
    }

//    Yolo yolo;
//    yolo.setConfigFilePath("/home/nvidia/Desktop/darknet/cfg/yolov2.cfg");
//    yolo.setDataFilePath("/home/nvidia/Desktop/darknet/cfg/coco.data");
//    yolo.setWeightFilePath("/home/nvidia/Desktop/darknet/yolov2.weights");
//    yolo.setNameListFile("/home/nvidia/Desktop/darknet/data/coco.names");
//    yolo.setThreshold(0.16);

    VideoCapture capture(argv[1]);
    if(!capture.isOpened())
    {
        cout << "cannot read video file" << std::endl;
        return 0;
    }

    Mat frame;
    Mat flow;
    UMat flowUmat, prevgray;
    while(true) {
        capture >> frame;
        if(frame.empty())
            break;

        resize(frame, frame, Size(600,400));

//        vector<DetectedObject> detection;
//        yolo.detect(frame, detection);
//
//        for(int i = 0; i < detection.size(); i++)
//        {
//            DetectedObject& o = detection[i];
//            rectangle(frame, o.bounding_box, Scalar(255,0,0), 2);
//
//            const char* class_name = yolo.getNames()[o.object_class];
//
//            char str[255];
//            sprintf(str,"%s", class_name);
//            putText(frame, str, Point2f(o.bounding_box.x,o.bounding_box.y), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0,0,255), 2);
//        }

        Mat img;
        frame.copyTo(img);

        // just make current frame gray
        cvtColor(img, img, COLOR_BGR2GRAY);

        if (prevgray.empty() == false ) {
            // calculate optical flow
            calcOpticalFlowFarneback(prevgray, img, flowUmat, 0.4, 1, 12, 2, 8, 1.2, 0);
            // copy Umat container to standard Mat
            flowUmat.copyTo(flow);
            // By y += 5, x += 5 you can specify the grid
            for (int y = 0; y < img.rows; y += 5) {
                for (int x = 0; x < img.cols; x += 5) {
                    // get the flow from y, x position * 10 for better visibility
                    const Point2f flowatxy = flow.at<Point2f>(y, x) * 10;
                    // draw line at flow direction
                    line(frame, Point(x, y), Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)),
                         Scalar(255, 0, 0));
                    // draw initial point
                    circle(frame, Point(x, y), 1, Scalar(0, 0, 0), -1);
                }
            }
            imshow("Demo", frame);
            waitKey(1);
        } else {
            img.copyTo(prevgray);
        }
    }
}
