#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <yolo.h>

int main(int argc, char** argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: %s <videofile>\n", argv[0]);
        return 0;
    }

    Yolo yolo;
    yolo.setConfigFilePath("/home/nvidia/Desktop/darknet/cfg/yolov2.cfg");
    yolo.setDataFilePath("/home/nvidia/Desktop/darknet/cfg/coco.data");
    yolo.setWeightFilePath("/home/nvidia/Desktop/darknet/yolov2.weights");
    yolo.setNameListFile("/home/nvidia/Desktop/darknet/data/coco.names");
    yolo.setThreshold(0.16);

    cv::VideoCapture capture(argv[1]);
    if(!capture.isOpened())
    {
        std::cout << "cannot read video file" << std::endl;
        return 0;
    }

//    cv::Mat img = cv::imread("/home/yildirim/Dropbox/tayse/deep/1/images/img_3.png");
    cv::Mat img;
    while(true)
    {
        capture >> img;
        if(img.empty())
            break;

        cv::resize(img, img, cv::Size(600,400));

        std::vector<DetectedObject> detection;
        yolo.detect(img, detection);

        for(int i = 0; i < detection.size(); i++)
        {
            DetectedObject& o = detection[i];
            cv::rectangle(img, o.bounding_box, cv::Scalar(255,0,0), 2);

            const char* class_name = yolo.getNames()[o.object_class];

            char str[255];
            //sprintf(str,"%s %f", names[o.object_class], o.prob);
            sprintf(str,"%s", class_name);
            cv::putText(img, str, cv::Point2f(o.bounding_box.x,o.bounding_box.y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0,0,255), 2);
        }

        cv::imshow("yolo demo", img);
        cv::waitKey(1);

    }
}
