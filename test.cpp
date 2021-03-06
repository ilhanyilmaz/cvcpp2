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
#include "haarcascade.cpp"

using namespace cv;
using namespace std;

bool tracking = false;
Mat mRgb, mHsv, mDiff, mTrack, mColorList;
int seekBarOffset = 10;
Mat * channels = new Mat[3];
Mat multiplied;
int displayChannelNo = 0;
int numDisplays = 8;
VideoCapture cap;
int blurType = 0;
int blurKernelSize = 5;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
    
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
        case 7:
			text = "track";
			break;
		default:
			text = "don't know";
			break;
	}
	putText( mat, text, Point(100,100), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255), 3, 8);
	imshow( windowName, mat );
	
}



void blurImg(Mat &img) {
    switch(blurType) {
        case 1:
            blur( img, img, Size( blurKernelSize, blurKernelSize ), Point(-1,-1) );
            break;
        case 2:
            GaussianBlur( img, img, Size( blurKernelSize, blurKernelSize ), 0, 0 );
            break;
        case 3:
            medianBlur ( img, img, blurKernelSize );
            break;
        case 4:
            //bilateralFilter ( img, img, blurKernelSize, blurKernelSize*2, blurKernelSize/2 );
            break;
    }
}

int main(int argc, char** argv )
{
	int key;
	Mat mObjPos(1,2,CV_8U);
	Mat prvs, next;
	Mat mFlow;
    Mat mTrack;
    Mat temp;

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

	Scalar color = Scalar( 255, 0, 0 );
    
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
    /// Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", "main", &lowThreshold, max_lowThreshold );
	//namedWindow( "flow1", 0 );
	//namedWindow( "flow2", 0 );
	
	setMouseCallback("main", mouseCallBackFunc, NULL);
	//init_cascade();
    
	while(true) {
		
		cap >> mRgb;
		cvtColor(mRgb, temp, CV_BGR2GRAY);
        resize(temp, next, Size(temp.cols/2, temp.rows/2));
        //detectAndDisplay(next );
        
        
		if(!prvs.empty()) {
            blurImg(next);
			mFlow = opticalFlowFarneback( prvs, next);
            
            Canny( mFlow, mFlow, lowThreshold, lowThreshold*ratio, kernel_size );
            
            findContours( mFlow, contours, hierarchy,
                CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
            
            
            /// Draw contours
            mTrack = next.clone();
            for( int i = 0; i< contours.size(); i++ ) {
                if(contourArea(contours[i])>100)
                    drawContours( mTrack, contours, i, color, 2, 8, hierarchy, 0, Point() );
            }
                
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
        else if (displayChannelNo == 7)
        	displayImage( "main", mTrack );
		else
			displayImage( "main", channels[displayChannelNo] );
			
		key = waitKey(30);
		if(key == 1048608) { // space key
			tracking = false;
			mColorList.release();
		}
        else if(key == 65363 || key == 1113939) { // 1113939
			displayChannelNo++;
			if(displayChannelNo==numDisplays)
				displayChannelNo=0;
		}
		else if(key == 65361 || key == 1113937) { // 1113937
			displayChannelNo--;
			if(displayChannelNo==-1)
				displayChannelNo=numDisplays - 1;
		}
        else if(key == 1048695) {
            blurType++;
            blurType = blurType % 5;
            printf("blurType: %i\n", blurType);
        }
        else if(key == 1048689) {
            blurType--;
            if(blurType == -1)
                blurType = 4;
            printf("blurType: %i\n", blurType);
        }
		else if(key >= 0) {
			printf("key pressed: %i\n", key);
			break;
		}
	}
	
	return 0;
	
}
