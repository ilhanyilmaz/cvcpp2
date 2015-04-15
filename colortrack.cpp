#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>


using namespace std;
using namespace cv;

int toHsv(Mat img, Mat& hsv);
int prepImg(Mat rgba, Mat& rgb, int width, int height);
int resizeImg(Mat src, Mat& dst, int width, int height);
int processDiffImg(Mat & diffImg);
int trackBiggestMovingObject(Mat & img, Mat & diffImg, Mat & trackImg, Mat & objectPos);
int trackColors(Mat img, Mat& diff, Mat colorList, int offset);


int resizeImg(Mat src, Mat& dst, int width, int height)
{
    resize(src, dst, Size(width, height));

    if (dst.rows == height && dst.cols == width)
    {
        return (1);
    }
    return(0);

}
int prepImg(Mat rgba, Mat& rgbSmall, int width, int height)
{
    Mat rgb;
    cvtColor(rgba, rgb, CV_RGBA2RGB); // Assuming RGBA input
    resize(rgb, rgbSmall, Size(width, height));

    //bgrBig.release();

    if (rgbSmall.rows == height && rgbSmall.cols == width)
    {
        return (height);
    }
    return(0);

}
int toHsv(Mat img, Mat& hsv)
{

    cvtColor(img, hsv, CV_BGR2HSV); // Assuming RGBA input

    if (hsv.rows == img.rows && hsv.cols == img.cols)
    {
        return (1);
    }
    return(0);
}



/*int removeBackground(Mat img, Mat& foreground, double color[3], int offset) {


    	Scalar lowerb = Scalar(color[0], color[1], color[2], 0);
    	Scalar upperb = Scalar(color[0], color[1], color[2], 255);

    	lowerb.val[0] = lowerb.val[0] - offset;
    	if(lowerb.val[0]<0)
    		lowerb.val[0] = 0;
    	upperb.val[0] = upperb.val[0] + offset;

    	lowerb.val[1] = lowerb.val[1] - offset;
    	if(lowerb.val[1]<0)
    		lowerb.val[1] = 0;
    	upperb.val[1] = upperb.val[1] + offset;
    	if(upperb.val[1]>255)
    		upperb.val[1] = 255;

    	lowerb.val[2] = lowerb.val[2] - offset;
    	if(lowerb.val[2]<0)
    		lowerb.val[2] = 0;
    	upperb.val[2] = upperb.val[2] + offset;
    	if(upperb.val[2]>255)
    		upperb.val[2] = 255;

    	//lowerb.val[2] = 0;
    	//upperb.val[2] = 255;

    	inRange(img, lowerb, upperb, foreground);
    	return (1);
}*/

int getMatWithColorMask(Mat img, Mat& colorMask,double color[3], int offset) {

    Scalar lowerb = Scalar(color[0], color[1], color[2], 0);
    Scalar upperb = Scalar(color[0], color[1], color[2], 255);

    lowerb.val[0] = lowerb.val[0] - offset;
    if(lowerb.val[0]<0)
        lowerb.val[0] = 0;
    upperb.val[0] = upperb.val[0] + offset;

    lowerb.val[1] = lowerb.val[1] - offset;
    if(lowerb.val[1]<0)
        lowerb.val[1] = 0;
    upperb.val[1] = upperb.val[1] + offset;
    if(upperb.val[1]>255)
        upperb.val[1] = 255;

    lowerb.val[2] = lowerb.val[2] - offset;
    if(lowerb.val[2]<0)
        lowerb.val[2] = 0;
    upperb.val[2] = upperb.val[2] + offset;
    if(upperb.val[2]>255)
        upperb.val[2] = 255;

    inRange(img, lowerb, upperb, colorMask);

    return 1;

}


int trackColors(Mat img, Mat& diff, Mat colorList, int offset)
{
    double color[3];
    Mat colorMask;

    if(colorList.rows <= 0)
        return 0;
    for(int i=0; i<colorList.rows; i++) {
        color[0] = (double)colorList.at<unsigned char>(i,0);
        color[1] = (double)colorList.at<unsigned char>(i,1);
        color[2] = (double)colorList.at<unsigned char>(i,2);

        getMatWithColorMask(img, colorMask, color, offset);

        //colorMask= getMatWithColorMask(img, color, offset);
        if(i==0)
            bitwise_or(colorMask, colorMask, diff);
        else
            bitwise_or(colorMask, diff, diff);
    }
    colorMask.release();
    return (1);
}

int processDiffImg(Mat & diffImg)
{


		Size kernelOpen(2,2);
        Size kernelClose(5,5);
        Mat element = getStructuringElement( MORPH_ELLIPSE, kernelOpen);


        erode(diffImg, diffImg, element);
        dilate(diffImg, diffImg, element);

        element = getStructuringElement( MORPH_RECT, kernelClose);
        morphologyEx( diffImg, diffImg, MORPH_CLOSE, element );

        element.release();
        //threshold(diffImg, diffImg, 1, 255, CV_THRESH_BINARY_INV);

        return (1);

}

int trackBiggestMovingObject(Mat & img, Mat & diffImg, Mat & trackImg, Mat & objPos)
{
    vector<vector<Point> > contours;
    double biggestContourArea = 0;
    double cContourArea = 0;
    Rect biggestContourRect(0,0,0,0);
    /// Detect edges using canny
    Canny( diffImg, trackImg, 100, 200, 3 );
    /// Find contours
    //findContours( trackImg, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    findContours( diffImg, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    //vector<Rect>::iterator it;
    //it = rectangles.begin();
    for( int i = 0; i< contours.size(); i++ )
    {
        cContourArea = contourArea(contours[i]);
        if(cContourArea<30)
            continue;
        //drawContours( frame, contours, i, 255 );

        if(cContourArea > biggestContourArea) {
            biggestContourArea = cContourArea;
            biggestContourRect = boundingRect(contours[i]);
            continue;
        }
    }
    objPos.at<unsigned char>(0,0) = (biggestContourRect.x + biggestContourRect.width/2) * 256 / diffImg.cols;
    objPos.at<unsigned char>(0,1) = (biggestContourRect.y + biggestContourRect.height/2) * 256 / diffImg.rows;

    rectangle(img, Point(biggestContourRect.x,biggestContourRect.y), Point(biggestContourRect.x+biggestContourRect.width,biggestContourRect.y+biggestContourRect.height),Scalar(255,0,0));
    if(biggestContourArea>0)
        return 1;
    else
        return 0;
}
