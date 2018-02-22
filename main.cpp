#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

int main(int argc, char** argv) {

    /** Stream Video **/
    // Open footage
    VideoCapture cap("../footage.mp4");
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1000);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600);

    // Check if footage opened succesfully
    if(!cap.isOpened()){
        cout << "Error opening video stream or file" << endl;
        return -1;
    }
    // Stream the footage
    while(1){
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break;
        resize(frame, frame, Size(1000, 600), 0, 0, INTER_CUBIC);
        imshow( "Footage", frame );
        // Press  ESC on keyboard to exit
        char c=(char)waitKey(25);
        if(c==27)
            break;
    }
    // Release the video capture object
    cap.release();
    // Closes all the frames
    destroyAllWindows();
    return 0;
}