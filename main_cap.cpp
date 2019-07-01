#include <string>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdlib.h>

#include "kcf.h"
using namespace std;
using namespace cv;

bool bRectLoaded = false;
void Callback(int event, int x, int y, int flags, void* userdata) {
    static int count = 0;
    if(count == 2) {
        if(!bRectLoaded)
            bRectLoaded = true;
        return;
    }

    if(event == EVENT_LBUTTONDOWN) {
        Rect& data = *(Rect*)(userdata);
        if(count == 0) {
            data.x = x;
            data.y = y;
        }
        else if(count == 1) {
            data.width = x - data.x;
            data.height = y - data.y;
        }
        ++count;
    }
}

int main(int argc, char const *argv[])
{
    VideoCapture cap;
    if (argc == 2) {
        cap.open(argv[1]);
        cout << "VideoCapture Input: " << argv[1] << endl;
    }
    else {
        cap.open(0);
        cout << "Open Camera 0" << endl;
    }

    if(!cap.isOpened()) {
        cout << "Open: " << argv[1] << " FAILED !" << endl;
        return -1;
    }
    
    KCF_Tracker tracker;
    namedWindow("Image");
    Rect init_rect;
    setMouseCallback("Image", Callback, &init_rect);
    // init_rect.x = 428;
    // init_rect.y = 272;
    // init_rect.width = 209;
    // init_rect.height = 273;

    Size videoSize = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH),(int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    cout << "Cap Size: " << videoSize << ", FOURCC: " << cap.get(CV_CAP_PROP_FOURCC)
    << ", FPS: " << cap.get(CV_CAP_PROP_FPS) <<  endl;
    
    VideoWriter writer;
    writer.open("output.mp4", cap.get(CV_CAP_PROP_FOURCC), cap.get(CV_CAP_PROP_FPS), videoSize);
    Mat frame;
    bool init_success = false;
    BBox_c bb;
    double avg_time = 0.;
    int frames = 0;
    while (1) {
        cap >> frame;
        if (frame.empty()) {
			cout << "Frame empty, exit...\n";
			break;
		}
        if (bRectLoaded) {
            if (!init_success) {
            cout << "tracker init" << endl;
            tracker.init(frame, init_rect);
            rectangle(frame, init_rect, Scalar( 0, 255, 0 ), 1, 8);
            init_success = true;
            }
            else {
                cout << "Keep tracking " << endl;
                double time_profile_counter = cv::getCPUTickCount();
                tracker.track(frame);
                time_profile_counter = cv::getCPUTickCount() - time_profile_counter;
                avg_time += time_profile_counter/((double)cvGetTickFrequency()*1000);
                frames++;
                bb = tracker.getBBox();
                rectangle(frame, cv::Rect(bb.cx - bb.w/2., bb.cy - bb.h/2., bb.w, bb.h), Scalar(0, 255, 0), 1, 8);
            }
        }
        writer << frame;
        imshow("Image", frame);
        waitKey(1);
    }
    cout << "Average processing speed " << avg_time/frames <<  "ms. (" << 1./(avg_time/frames)*1000 << " fps)" << endl;
    return 0;
}
