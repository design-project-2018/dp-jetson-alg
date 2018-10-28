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

int frame_width = 224;
int frame_height = 224;

/**
 * Object extractor performs the following functions
 * 1) Loops through all videos in an input directory
 * 2) Performs YOLO detection for each frame
 * 3) Outputs object coordinates to a json file
 *
 * @param argc
 * @param argv
 * @return
 */
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

    // to store current frame
    Mat frame;

    // instantiate yolo functions
    Yolo yolo;
    yolo.setConfigFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/cfg/yolov2.cfg");
    yolo.setDataFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/cfg/coco.data");
    yolo.setWeightFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/yolov2.weights");
    yolo.setNameListFile("/home/nvidia/Desktop/dp-jetson-alg/darknet/data/coco.names");
    yolo.setThreshold(0.3);

    // keep track of frame number
    int frame_num = 0;

    // open file stream for writing result to
    ofstream result;
    result.open(argv[2] + filename.substr(0, filename.size()-4) + ".json");
    result << "{" << std::endl;
    result << "    \"file\": \"" << filename << "\"," << endl;
    result << "    \"frames\": [" << endl;

    // repeatedly do computations for each frame
    while(true) {

        // capture frame
        capture >> frame;
        frame_num++;
        if(frame.empty())
            break;

        cout << "Extracting objects from " << filename << ". Frame - " << frame_num << endl;

        // resize frame
        resize(frame, frame, Size(frame_width,frame_height));

        // perform yolo
        vector<DetectedObject> detection;
        yolo.detect(frame, detection);

        result << "        {" << std::endl;
        result << "            \"id\": " << frame_num << "," << endl;
        result << "            \"objects\": [" << endl;

        // write results to file stream
        for(int i = 0; i < detection.size(); i++) {
            DetectedObject &o = detection[i];

            result << "                {" << std::endl;
            result << "                    \"id\": " << i << "," << endl;
            result << "                    \"type\": " << o.object_class << "," << endl;
            result << "                    \"probability\": " << o.prob << "," << endl;
            result << "                    \"x\": " << int(o.bounding_box.x) << "," << endl;
            result << "                    \"y\": " << int(o.bounding_box.y) << "," << endl;
            result << "                    \"width\": " << int(o.bounding_box.width) << "," << endl;
            result << "                    \"height\": " << int(o.bounding_box.height) << endl;
            if (i == detection.size() - 1) {
                result << "                }" << endl;
            } else {
                result << "                }," << endl;
            }
        }
        result << "            ]" << endl;
        if (frame_num == 100) {
            result << "        }" << endl;
        } else {
            result << "        }," << endl;
        }
    }
    // finish writing to file
    result << "    ]" << endl;
    result << "}" << std::endl;
    result.close();
}
