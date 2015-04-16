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
   
Mat opticalFlowFarneback( Mat prvs, Mat next) {
    Mat flow;
    Mat mag, angle;
    Mat hsv, rgb;
    Mat * flowCh = new Mat[2];
    Mat * hsvCh = new Mat[3];
    
    calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 25, 3, 5, 1.2, 0);
    split(flow, flowCh);
    cartToPolar(flowCh[0], flowCh[1], mag, angle, true);
    
    hsvCh[0]=angle;
    hsvCh[1]=Mat::ones(mag.rows, mag.cols, CV_32F);
    //hsvCh[2]= mag;
    normalize(mag, hsvCh[2], 0, 1.0, NORM_MINMAX);
    
    
    merge(hsvCh, 3, hsv);
    cvtColor(hsv, rgb, CV_HSV2BGR);
    
    //imshow("flow1", rgb);
    //imshow("flow2", flowCh[1]);
    //Mat cflow;
    //cvtColor(prvs, cflow, CV_GRAY2BGR);
    
    //drawOptFlowMap(flow, cflow, 10, CV_RGB(0, 255, 0));
    return rgb;
}
