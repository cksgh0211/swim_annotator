// VideoStream.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp> 
#include <iostream>

using namespace std;
using namespace cv;

VideoCapture createVideoStream(string videoFile);
void playVideo(VideoCapture videoStream, int frameDelay);
void closeVideo(VideoCapture videoStream);


int main(int argc, char* argv[])
{
    //string name = "0.avi";
//    VideoCapture videoStream = createVideoStream(argv[1]);
   //playVideo(videoStream);

    VideoCapture videoStream(argv[1]);

    int frameDelay = 1;
    if (argc > 3) {
        frameDelay = (int) argv[2];
    }

    playVideo(videoStream,frameDelay);

    return 0;
}

VideoCapture createVideoStream(string videoFile) {

    VideoCapture videoStream(videoFile);

    if (!videoStream.isOpened()) {

        cout << "Error opening video stream or file" << endl;

        return -1;
    }
    else
    {
        videoStream;
    }
}

void playVideo(VideoCapture videoStream, int frameDelay) {
    
    while (1) {
        // Capture frame-by-frame
        Mat frame;
        videoStream >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
            break;

        // Display the resulting frame
        imshow("Frame", frame);


        // Delay frames
        char c = (char)waitKey(frameDelay);
        
        // Press  ESC on keyboard to exit
        if (c == 27)
            break;

    }
    closeVideo(videoStream);
}

void closeVideo(VideoCapture videoStream) {
    destroyAllWindows();
    videoStream.release();
}