#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

int frame_width = 224;
int frame_height = 224;

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

    // extract name of file
    string path = argv[1];
    string filename = path.substr(path.find_last_of("/\\")+1);

    cout << "Creating optical flow mask for file: " << filename << endl;

    // to store current frame
    Mat frame;

    // instantiate farneback objects
    Mat flow;
    UMat flowUmat, prevgray;

    // repeatedly do computations for each frame
    while(true) {

        // capture frame
        capture >> frame;
        if(frame.empty())
            break;

        // resize frame
        resize(frame, frame, Size(frame_width,frame_height));

        // flow mask frame
        Mat flowMask = Mat(frame_width, frame_height, CV_8UC3, Scalar(255,255,255));

        // frames
        Mat originalFrame;
        Mat flowFrame;
        frame.copyTo(originalFrame);
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

            // create flow mask
            for (int y = 0; y < frame_width; y++) {
                for (int x = 0; x < frame_width; x++) {
                    // get the flow from y, x position * 10 for better visibility
                    const Point2f flowatxy = flow.at<Point2f>(y, x) * 10;
                    double xVector = flowatxy.x + 128;
                    double yVector = flowatxy.y + 128;
                    double magnitude = sqrt(pow((xVector - 128), 2) + pow((yVector - 128), 2));// sqrt[(x-128)^2+(y-128)^2]
                    flowMask.at<cv::Vec3b>(y,x)[0] = xVector;
                    flowMask.at<cv::Vec3b>(y,x)[1] = yVector;
                    flowMask.at<cv::Vec3b>(y,x)[2] = magnitude;
                }
            }
            // display
            displayFrames("YOLO and Optical Flow", 4, originalFrame, flowMask, flowFrame, flowMask);
        } else {
            img.copyTo(prevgray);
        }
    }
}
