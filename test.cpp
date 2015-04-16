#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <stdio.h>
#include "colortrack.cpp"
#include "opticalFlowFarneback.cpp"
#include "histogram.cpp"

using namespace cv;
using namespace std;

bool tracking = false;
Mat mRgb, mHsv, mDiff, mTrack, mColorList;
int seekBarOffset = 10;
Mat * channels = new Mat[3];
Mat multiplied;
int displayChannelNo = 0;
int numDisplays = 7;
VideoCapture cap;

void mouseCallBackFunc(int event, int x, int y, int flags, void* userdata){
	if(event == EVENT_LBUTTONDOWN) {
		tracking = true;
		if(mColorList.empty()) {
			//mColorList = new Mat( 1, 3, CV_8UC3);
		}
		uint8_t* pixelPtr = (uint8_t*)mHsv.data;
		
		Mat color( 1, 3, CV_8UC3);
		color.at<unsigned char>(0,0) = pixelPtr[y*mHsv.cols*3+x*3];
		color.at<unsigned char>(0,1) = pixelPtr[y*mHsv.cols*3+x*3+1];
		color.at<unsigned char>(0,2) = pixelPtr[y*mHsv.cols*3+x*3+2];
		mColorList.push_back(color);
		printf("point: %i : %i\n",x,y);
	}
}
void displayImage(string windowName, Mat mat) {
	string text;
	switch(displayChannelNo) {
		case 0:
			text = "hue";
			break;
		case 1:
			text = "saturation";
			break;
		case 2:
			text = "value";
			break;
		case 3:
			text = "multiplied";
			break;
		case 4:
			text = "hsv";
			break;
		case 5:
			text = "rgb";
			break;
		case 6:
			text = "flow";
			break;
		default:
			text = "don't know";
			break;
	}
	putText( mat, text, Point(100,100), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255), 3, 8);
	imshow( windowName, mat );
	
}



int main(int argc, char** argv )
{
	int key;
	Mat mObjPos(1,2,CV_8U);
	Mat prvs, next;
	Mat mFlow;
    Mat temp;
	
	if ( argc != 2 )
    {
		cap.open(0);
        //printf("usage: DisplayImage.out <Image_Path>\n");
        //return -1;
    }
    else
		cap.open(argv[1]);
	
	//VideoCapture cap( argv[1] );
	//VideoCapture cap( 0 );
	
	namedWindow( "main", 0 );
	//namedWindow( "flow1", 0 );
	//namedWindow( "flow2", 0 );
	
	setMouseCallback("main", mouseCallBackFunc, NULL);
	
    
	while(true) {
		
		cap >> mRgb;
		cvtColor(mRgb, temp, CV_BGR2GRAY);
        resize(temp, next, Size(temp.cols/2, temp.rows/2));
        
		if(!prvs.empty()) {
			mFlow = opticalFlowFarneback( prvs, next);
			prvs.release();
		}
		prvs = next.clone();
		
		toHsv(mRgb, mHsv);
		split(mHsv, channels);
		addWeighted(channels[0], 0.5, channels[1], 0.5, 0, multiplied);
		
		//drawHistogram(mHsv);
		
		if(tracking) {
			trackColors(mHsv, mDiff, mColorList, seekBarOffset);
			processDiffImg(mDiff);
			trackBiggestMovingObject(mRgb, mDiff, mTrack, mObjPos);
			displayImage("main", mRgb);
		}
		else if (displayChannelNo == 3)
		displayImage("main", multiplied);
		else if (displayChannelNo == 4)
			displayImage( "main", mHsv );
		else if (displayChannelNo == 5)
			displayImage( "main", mRgb );
		else if (displayChannelNo == 6)
			displayImage( "main", mFlow );
		else
			displayImage( "main", channels[displayChannelNo] );
			
		key = waitKey(30);
		if(key == 1048608) { // space key
			tracking = false;
			mColorList.release();
		}
		else if(key == 1113939) {
			displayChannelNo++;
			if(displayChannelNo==numDisplays)
				displayChannelNo=0;
		}
		else if(key == 1113937) {
			displayChannelNo--;
			if(displayChannelNo==-1)
				displayChannelNo=numDisplays - 1;
		}
		else if(key >= 0) {
			printf("key pressed: %i\n", key);
			break;
		}
	}
	
	return 0;
	
}
