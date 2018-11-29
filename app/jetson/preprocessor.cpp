#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <unistd.h>

#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include <yolo.h>

using namespace cv;
using namespace std;

int frame_width = 224;
int frame_height = 224;

typedef websocketpp::client<websocketpp::config::asio_client> client;

class connection_metadata {
    public:
        typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

        connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri)
        : m_id(id)
        , m_hdl(hdl)
        , m_status("Connecting")
        , m_uri(uri)
        , m_server("N/A")
        {}

        void on_open(client * c, websocketpp::connection_hdl hdl) {
            m_status = "Open";

            client::connection_ptr con = c->get_con_from_hdl(hdl);
            m_server = con->get_response_header("Server");
        }

        void on_fail(client * c, websocketpp::connection_hdl hdl) {
            m_status = "Failed";

            client::connection_ptr con = c->get_con_from_hdl(hdl);
            m_server = con->get_response_header("Server");
            m_error_reason = con->get_ec().message();
        }
        
        void on_close(client * c, websocketpp::connection_hdl hdl) {
            m_status = "Closed";
            client::connection_ptr con = c->get_con_from_hdl(hdl);
            std::stringstream s;
            s << "close code: " << con->get_remote_close_code() << " (" 
            << websocketpp::close::status::get_string(con->get_remote_close_code()) 
            << "), close reason: " << con->get_remote_close_reason();
            m_error_reason = s.str();
        }

        void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
            if (msg->get_opcode() == websocketpp::frame::opcode::text) {
                m_messages.push_back("<< " + msg->get_payload());
            } else {
                m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
            }
        }

        websocketpp::connection_hdl get_hdl() const {
            return m_hdl;
        }
        
        int get_id() const {
            return m_id;
        }
        
        std::string get_status() const {
            return m_status;
        }

        void record_sent_message(std::string message) {
            m_messages.push_back(">> " + message);
        }

        friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data);
    private:
        int m_id;
        websocketpp::connection_hdl m_hdl;
        std::string m_status;
        std::string m_uri;
        std::string m_server;
        std::string m_error_reason;
        std::vector<std::string> m_messages;
};

std::ostream & operator<< (std::ostream & out, connection_metadata const & data) {
    out << "> URI: " << data.m_uri << "\n"
        << "> Status: " << data.m_status << "\n"
        << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n";
    out << "> Messages Processed: (" << data.m_messages.size() << ") \n";

    std::vector<std::string>::const_iterator it;
    for (it = data.m_messages.begin(); it != data.m_messages.end(); ++it) {
        out << *it << "\n";
    }

    return out;
}

class websocket_endpoint {
    public:
        websocket_endpoint () : m_next_id(0) {
            m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
            m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

            m_endpoint.init_asio();
            m_endpoint.start_perpetual();

            m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
        }

        ~websocket_endpoint() {
            m_endpoint.stop_perpetual();
            
            for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
                if (it->second->get_status() != "Open") {
                    // Only close open connections
                    continue;
                }
                
                std::cout << "> Closing connection " << it->second->get_id() << std::endl;
                
                websocketpp::lib::error_code ec;
                m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
                if (ec) {
                    std::cout << "> Error closing connection " << it->second->get_id() << ": "  
                            << ec.message() << std::endl;
                }
            }
            
            m_thread->join();
        }

        int connect(std::string const & uri) {
            websocketpp::lib::error_code ec;

            client::connection_ptr con = m_endpoint.get_connection(uri, ec);

            if (ec) {
                std::cout << "> Connect initialization error: " << ec.message() << std::endl;
                return -1;
            }

            int new_id = m_next_id++;
            connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id, con->get_handle(), uri);
            m_connection_list[new_id] = metadata_ptr;

            con->set_open_handler(websocketpp::lib::bind(
                &connection_metadata::on_open,
                metadata_ptr,
                &m_endpoint,
                websocketpp::lib::placeholders::_1
            ));
            con->set_fail_handler(websocketpp::lib::bind(
                &connection_metadata::on_fail,
                metadata_ptr,
                &m_endpoint,
                websocketpp::lib::placeholders::_1
            ));
            con->set_close_handler(websocketpp::lib::bind(
                &connection_metadata::on_close,
                metadata_ptr,
                &m_endpoint,
                websocketpp::lib::placeholders::_1
            ));
            con->set_message_handler(websocketpp::lib::bind(
                &connection_metadata::on_message,
                metadata_ptr,
                websocketpp::lib::placeholders::_1,
                websocketpp::lib::placeholders::_2
            ));

            m_endpoint.connect(con);

            return new_id;
        }

        void close(int id, websocketpp::close::status::value code, std::string reason) {
            websocketpp::lib::error_code ec;
            
            con_list::iterator metadata_it = m_connection_list.find(id);
            if (metadata_it == m_connection_list.end()) {
                std::cout << "> No connection found with id " << id << std::endl;
                return;
            }
            
            m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
            if (ec) {
                std::cout << "> Error initiating close: " << ec.message() << std::endl;
            }
        }

        void send(int id, std::string message) {
            websocketpp::lib::error_code ec;
            
            con_list::iterator metadata_it = m_connection_list.find(id);
            if (metadata_it == m_connection_list.end()) {
                std::cout << "> No connection found with id " << id << std::endl;
                return;
            }
            
            m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
            if (ec) {
                std::cout << "> Error sending message: " << ec.message() << std::endl;
                return;
            }
            
            metadata_it->second->record_sent_message(message);
        }

        connection_metadata::ptr get_metadata(int id) const {
            con_list::const_iterator metadata_it = m_connection_list.find(id);
            if (metadata_it == m_connection_list.end()) {
                return connection_metadata::ptr();
            } else {
                return metadata_it->second;
            }
        }
    private:
        typedef std::map<int,connection_metadata::ptr> con_list;

        client m_endpoint;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

        con_list m_connection_list;
        int m_next_id;
};

int main(int argc, char** argv) {

    string output_folder = "/home/nvidia/Downloads/output/";

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

    // connect to websocket
    websocket_endpoint endpoint;
    int id = endpoint.connect("ws://localhost:8000");
    if (id != -1) {
        cout << "> Created connection with id " << id << endl;
    }
    usleep(1000000);

    // to store current frame
    Mat frame;

    // instantiate farneback objects
    Mat flow;
    UMat flowUmat, prevgray;

    // instantiate yolo functions
    Yolo yolo;
    yolo.setConfigFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/cfg/yolov2.cfg");
    yolo.setDataFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/cfg/coco.data");
    yolo.setWeightFilePath("/home/nvidia/Desktop/dp-jetson-alg/darknet/yolov2.weights");
    yolo.setNameListFile("/home/nvidia/Desktop/dp-jetson-alg/darknet/data/coco.names");
    yolo.setThreshold(0.3);

    // keep track of frame number
    int frame_num = 0;

    // repeatedly do computations for each frame
    while(true) {

        // capture frame
        capture >> frame;
        frame_num++;
        if(frame.empty())
            break;

        // resize frame
        resize(frame, frame, Size(frame_width,frame_height));

        // open stream for writing packet data to be sent to web socket
        stringstream packet;
        packet << "{";
        packet << "\"frame_id\":" << frame_num << ",";

        // path to original frame
        string file = output_folder + to_string(frame_num) + "-original.jpg";
        packet << "\"original_frame\":\"" << file << "\",";
        imwrite( file, frame );

        // perform optical flow
        Mat flowMask = Mat(frame_width, frame_height, CV_8UC3, Scalar(255,255,255));

        Mat originalFrame;
        Mat flowFrame;
        Mat yoloFrame;
        frame.copyTo(originalFrame);
        frame.copyTo(flowFrame);
        frame.copyTo(yoloFrame);

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
        // path to flow frame
        file = output_folder + to_string(frame_num) + "-flow.jpg";
        packet << "\"flow_frame\":\"" << file << "\",";
        imwrite( file, flowMask );

        // perform yolo
        vector<DetectedObject> detection;
        yolo.detect(frame, detection);
        packet << "\"objects\":[" << endl;

        // write results to file stream
        for(int i = 0; i < detection.size(); i++) {
            DetectedObject &o = detection[i];

            // draw object boxes
            rectangle(yoloFrame, o.bounding_box, Scalar(0, 0, 255), 2);
            const char *class_name = yolo.getNames()[o.object_class];
            char str[255];
            sprintf(str, "%s", class_name);
            putText(yoloFrame, str, Point2f(o.bounding_box.x, o.bounding_box.y), FONT_HERSHEY_SIMPLEX, 0.6,
                    Scalar(0, 0, 255), 2);

            packet << "{";
            packet << "\"id\": " << i << ",";
            packet << "\"type\": " << o.object_class << ",";
            packet << "\"probability\": " << o.prob << ",";
            packet << "\"x\": " << int(o.bounding_box.x) << ",";
            packet << "\"y\": " << int(o.bounding_box.y) << ",";
            packet << "\"width\": " << int(o.bounding_box.width) << ",";
            packet << "\"height\": " << int(o.bounding_box.height);
            if (i == detection.size() - 1) {
                packet << "}";
            } else {
                packet << "},";
            }
        }
        packet << "],";

        // path to yolo frame
        file = output_folder + to_string(frame_num) + "-yolo.jpg";
        packet << "\"yolo_frame\":\"" << file << "\"";
        imwrite( file, yoloFrame );

        packet << "}" << endl;

        cout << "Sending result for frame: " << frame_num << endl;
        endpoint.send(id, packet.str());
    }

    usleep(3000000);

    // close connection
    cout << "Closing web socket connection." << endl;
    int close_code = websocketpp::close::status::normal;
    string reason = "Ending process";
    endpoint.close(id, close_code, reason);

    usleep(3000000);
    return 0;
}