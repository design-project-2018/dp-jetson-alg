#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>

#include <yolo.h>

using namespace cv;
using namespace std;

int FRAME_FREQ = 4;

/**
 * Displays muliple frames in the same window
 *
 * @param title
 * @param nArgs
 */
void displayFrames(string title, int nArgs, ...) {
    int size;
    int i;
    int m, n;
    int x, y;

    // w - Maximum number of images in a row
    // h - Maximum number of images in a column
    int w , h;

    // scale - How much we have to resize the image
    float scale;
    int max;

    // If the number of arguments is lesser than 0 or greater than 12
    // return without displaying
    if(nArgs <= 0) {
        printf("Number of arguments too small....\n");
        return;
    }
    else if(nArgs > 14) {
        printf("Number of arguments too large, can only handle maximally 12 images at a time ...\n");
        return;
    }
    // Determine the size of the image,
    // and the number of rows/cols
    // from number of arguments
    else if (nArgs == 1) {
        w = h = 1;
        size = 500;
    }
    else if (nArgs == 2) {
        w = 2; h = 1;
        size = 500;
    }
    else if (nArgs == 3 || nArgs == 4) {
        w = 2; h = 2;
        size = 500;
    }
    else if (nArgs == 5 || nArgs == 6) {
        w = 3; h = 2;
        size = 400;
    }
    else if (nArgs == 7 || nArgs == 8) {
        w = 4; h = 2;
        size = 400;
    }
    else {
        w = 4; h = 3;
        size = 350;
    }

    // Create a new 3 channel image
    Mat DispImage = Mat::zeros(Size(100 + size*w, 60 + size*h), CV_8UC3);

    // Used to get the arguments passed
    va_list args;
    va_start(args, nArgs);

    // Loop for nArgs number of arguments
    for (i = 0, m = 20, n = 20; i < nArgs; i++, m += (20 + size)) {
        // Get the Pointer to the IplImage
        Mat img = va_arg(args, Mat);

        // Check whether it is NULL or not
        // If it is NULL, release the image, and return
        if(img.empty()) {
            printf("Invalid arguments");
            return;
        }

        // Find the width and height of the image
        x = img.cols;
        y = img.rows;

        // Find whether height or width is greater in order to resize the image
        max = (x > y)? x: y;

        // Find the scaling factor to resize the image
        scale = (float) ( (float) max / size );

        // Used to Align the images
        if( i % w == 0 && m!= 20) {
            m = 20;
            n+= 20 + size;
        }

        // Set the image ROI to display the current image
        // Resize the input image and copy the it to the Single Big Image
        Rect ROI(m, n, (int)( x/scale ), (int)( y/scale ));
        Mat temp; resize(img,temp, Size(ROI.width, ROI.height));
        temp.copyTo(DispImage(ROI));
    }

    // Create a new window, and show the Single Big Image
    namedWindow( title, 1 );
    imshow( title, DispImage);
    waitKey(1);

    // End the number of arguments
    va_end(args);
}

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
    yolo.setConfigFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/cfg/yolov2.cfg");
    yolo.setDataFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/cfg/coco.data");
    yolo.setWeightFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/yolov2.weights");
    yolo.setNameListFile("/home/nvidia/Desktop/dp-jetson-alg/darknet/data/coco.names");
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
            displayFrames("YOLO and Optical Flow", 4, originalFrame, yoloFrame, flowFrame, frame);
        } else {
            img.copyTo(prevgray);
        }
    }
}
