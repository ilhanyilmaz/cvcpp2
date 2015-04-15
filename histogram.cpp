#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <stdio.h>

Mat histImg;
/*void histogramCallBackFunc(int event, int x, int y, int flags, void* userdata){
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
}*/

void drawHistogram(Mat &hsv) {
	int hbins = 30, sbins = 32;
    int histSize[] = {hbins, sbins};
    // hue varies from 0 to 179, see cvtColor
    float hranges[] = { 0, 180 };
    // saturation varies from 0 (black-gray-white) to
    // 255 (pure spectrum color)
    float sranges[] = { 0, 256 };
    const float* ranges[] = { hranges, sranges };
    MatND hist;
    // we compute the histogram from the 0-th and 1-st channels
    int channels[] = {0, 1};

    calcHist( &hsv, 1, channels, Mat(), // do not use mask
             hist, 2, histSize, ranges,
             true, // the histogram is uniform
             false );
    double maxVal=0;
    minMaxLoc(hist, 0, &maxVal, 0, 0);

    int scale = 10;
    histImg = Mat::zeros(sbins*scale, hbins*10, CV_8UC3);

    for( int h = 0; h < hbins; h++ )
        for( int s = 0; s < sbins; s++ )
        {
            float binVal = hist.at<float>(h, s);
            int intensity = cvRound(binVal*255/maxVal);
            rectangle( histImg, Point(h*scale, s*scale),
                        Point( (h+1)*scale - 1, (s+1)*scale - 1),
                        Scalar::all(intensity),
                        CV_FILLED );
        }

    namedWindow( "histogram", 1 );
    
    //setMouseCallback("histogram", histogramCallBackFunc, NULL);
    
    imshow( "histogram", histImg );
}
