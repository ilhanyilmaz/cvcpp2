#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <stdio.h>

using namespace cv;
using namespace std;


void drawOptFlowMap (const Mat& flow, Mat& cflowmap, int step, const Scalar& color) {  
 for(int y = 0; y < cflowmap.rows; y += step)  
        for(int x = 0; x < cflowmap.cols; x += step)  
        {  
            const Point2f& fxy = flow.at< Point2f>(y, x);  
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)),  
                 color);  
            circle(cflowmap, Point(cvRound(x+fxy.x), cvRound(y+fxy.y)), 1, color, -1);  
        }
    }
   
Mat flow;

Mat opticalFlowFarneback( Mat prvs, Mat next) {
    
    Mat mag, angle;
    Mat hsv, rgb;
    Mat * flowCh = new Mat[2];
    Mat * hsvCh = new Mat[3];
    
    calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0 );
    //calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 7, 1.5, OPTFLOW_USE_INITIAL_FLOW );
    split(flow, flowCh);
    cartToPolar(flowCh[0], flowCh[1], mag, angle, false);
    
    normalize(angle, angle, 0, 1.0, NORM_MINMAX);
    mag.convertTo(hsvCh[2], CV_8U, 255, 0);
    angle.convertTo(hsvCh[0], CV_8U, 180);
    hsvCh[1]=Mat(hsvCh[0].rows, hsvCh[0].cols, CV_8U, Scalar(255));
    
    merge(hsvCh, 3, hsv);
    cvtColor(hsv, rgb, CV_HSV2BGR);
    
    return hsvCh[2];
}

/*
    hsvCh[0]=angle;
    hsvCh[0].convertTo(hsvCh[0], CV_8U, 255, 0);
    //normalize(angle, hsvCh[0], 0, 1.0, NORM_MINMAX);
    hsvCh[1]=Mat::ones(mag.rows, mag.cols, CV_8U);
    //hsvCh[2]= mag;
    normalize(mag, hsvCh[2], 0, 1.0, NORM_MINMAX);
    hsvCh[2].convertTo(hsvCh[2], CV_8U, 255, 0);
    
    merge(hsvCh, 3, hsv);
    
    //hsv.convertTo(hsv, CV_8UC3, 255, 0);
    cvtColor(hsv, rgb, CV_HSV2BGR);
*/
