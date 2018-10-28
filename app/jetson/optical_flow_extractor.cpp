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

    // create output video
    const string outputName = argv[2] + filename;
    int ex = static_cast<int>(capture.get(CAP_PROP_FOURCC));
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    Size size = Size(frame_width, frame_height);
    VideoWriter outputVideo;
    outputVideo.open(outputName, ex, capture.get(CAP_PROP_FPS), size, true);
    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for write: " << argv[1] << endl;
        return -1;
    }

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
        } else {
            img.copyTo(prevgray);
        }
        outputVideo << flowMask;
    }
}
