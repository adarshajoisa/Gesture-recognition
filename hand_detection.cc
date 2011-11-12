//Hand movement recognition
//Abhishek Anupam, Adarsha Joisa
//Movement controls are working, but they need to get better. The down key behaves pressed most of the time

#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "math.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <time.h>

using namespace std;

int main()
{
  int c = 0;
  int prevx = 0, prevy = 0, curx = 0, cury = 0;
  int prevheight = 0, prevwidth = 0, curheight = 0, curwidth = 0;
  int initx = 0, inity = 0, initheight = 0, initwidth = 0;
  int framecount = 0;
  CvSeq* a = 0;
  CvCapture* capture = cvCaptureFromCAM(0);		//capture video from camera
  if(!cvQueryFrame(capture)){ cout<<"Video capture failed, please check the camera."<<endl;}else{cout<<"Video camera capture status: OK"<<endl;};
  CvSize sz = cvGetSize(cvQueryFrame( capture));	//get frame size
  
  //create intermediate images for processing
  IplImage* src = cvCreateImage( sz, 8, 3 );
  IplImage* hsv_image = cvCreateImage( sz, 8, 3);
  IplImage* hsv_mask = cvCreateImage( sz, 8, 1);
  IplImage* hsv_edge = cvCreateImage( sz, 8, 1);
  
  //min and max values for converting image into binary
  CvScalar  hsv_min = cvScalar(0, 30, 80, 0);
  CvScalar  hsv_max = cvScalar(20, 150, 255, 0);
  CvMemStorage* storage = cvCreateMemStorage(0);
  CvMemStorage* areastorage = cvCreateMemStorage(0);
  CvMemStorage* minStorage = cvCreateMemStorage(0);
  CvMemStorage* dftStorage = cvCreateMemStorage(0);
  CvSeq* contours = NULL;	//sequential array-like storage
  cvNamedWindow( "src",1);	//window to display source video
  
  
  /* code to initialize the 4 variables: prev* starts here. */
  
  
  
  
  //Draw grid for the output window
  /*for(int b = 0; b< int(bg->width/10); b++)
  {
    cvLine( bg, cvPoint(b*20, 0), cvPoint(b*20, bg->height), CV_RGB( 200, 200, 200), 1, 8, 0 );
    cvLine( bg, cvPoint(0, b*20), cvPoint(bg->width, b*20), CV_RGB( 200, 200, 200), 1, 8, 0 );
  }
  */
//   continuescan1:
  cout<<"Waiting for hand..."<<endl;
  while(initwidth < 125 || initheight < 125 || framecount < 2)
  {
    IplImage* bg = cvCreateImage( sz, 8, 3);
    cvRectangle( bg, cvPoint(bg->width,bg->height), cvPoint(0,0), CV_RGB( 255, 255, 255), -1, 8, 0 );
    bg->origin = 1;
    src = cvQueryFrame( capture);		//capture frame from camera input video
    framecount++;
    cvCvtColor(src, hsv_image, CV_BGR2HSV);	//convert from rgb to hsv
    cvInRangeS (hsv_image, hsv_min, hsv_max, hsv_mask);	//convert frame into binary (hsv_image -> hsv_mask
    cvSmooth( hsv_mask, hsv_mask, CV_MEDIAN, 27, 0, 0, 0 );	//use median smoothing to remove noise
    cvCanny(hsv_mask, hsv_edge, 1, 3, 5);		//Canny edge detector for edge detection
    cvFindContours( hsv_mask, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	//Find contours in the frame
    CvSeq* contours2 = NULL;
    double result = 0, result2 = 0;
    
    while(contours)		//find the contour with the largest area
    {
      result = fabs( cvContourArea( contours, CV_WHOLE_SEQ ) );	//calculate area of the contour
      if ( result > result2) {result2 = result; contours2 = contours;};
	contours  =  contours->h_next;
    }
    
    if ( contours2 )
    {
      CvRect rect = cvBoundingRect( contours2, 0 );		//get the bounding rectangle for the largest contour
      //draw the bounding rectangle on the output window
      cvRectangle( bg, cvPoint(rect.x, rect.y + rect.height), cvPoint(rect.x + rect.width, rect.y), CV_RGB(200, 0, 200), 1, 8, 0 );
      int checkcxt = cvCheckContourConvexity( contours2 );		//check if the contour is convex
      CvSeq* hull = cvConvexHull2( contours2, 0, CV_CLOCKWISE, 0 );	//draw a convex hull around the contour
      CvSeq* defect = cvConvexityDefects( contours2, hull, dftStorage );	//return convexity defects in the contour
      
      /*
      if( defect->total >=40 ) 
	cout << " Closed Palm " << endl;
      else if( defect->total >=30 && defect->total <40 ) 
	cout << " Open Palm " << endl;
      else
      cout << " Fist " << endl;
      
      
      cout << "defect: " << defect->total << endl;*/
      CvBox2D box = cvMinAreaRect2( contours2, minStorage );	//circumscribe a rectangle around the contour
      initx = curx = box.center.x;
      inity = cury = box.center.y;
      initheight = curheight = box.size.height;
      initwidth = curwidth = box.size.width;
//       cvDrawContours( bg, contours2,  CV_RGB( 0, 200, 0), CV_RGB( 0, 100, 0), 1, 1, 8, cvPoint(0,0));	//draw the contour in the output window
// 	cvShowImage( "src", src);
// 	cvNamedWindow("bg",0); 
// 	cvShowImage("bg",bg); 
// 	cvReleaseImage( &bg);
// 	c = cvWaitKey(100);
  //      if(initwidth < 175 || initheight < 175 || framecount < 2)	//ignore if there's no hand in the frame
  //       goto continuescan1;
    }
  }
    
  //initialization code ends here
    
  int sideflag = 0;		//this flag decides whether the hand has moved sideways relative to its initial position
  int upflag = 0;		//this flag decides whether the hand has moved forward/backward relative to its initial position
  int left = 0, right = 0, up = 0, down = 0;	//flags to decide direction of movement
  cout<<initheight<<"\t"<<initwidth<<endl;
  cout<<initx<<"\t"<<inity<<endl;
  cout<<endl<<"Hand detected. Starting tracking."<<endl;
  while( c != 27)
  {
    if(curwidth < 125 || curheight < 125 || framecount < 2)	//ignore if there's no hand in the frame
    {
      system("xdotool search --name DOSBox keyup Left");
      system("xdotool search --name DOSBox keyup Down");
      system("xdotool search --name DOSBox keyup Right");
      system("xdotool search --name DOSBox keyup Up");
      goto continuescan;
    }
    //following is the logic to get directions from hand position
      if(curx > (initx - 25) || curx < (initx + 25))
      {
	sideflag = 0;
	left = right = 0;
      }
      if(((curheight > (initheight - 75)) && (curheight < (initheight + 75))) && ((curwidth > (initwidth - 75)) && (curwidth < (initwidth + 75))))
      {
	upflag = 0;
	up = down = 0;
      }
      
      if(curx < (initx - 50)) 	//the output image is inverted, so left in the x-axis translates to an actual right movement
      {
	sideflag = 1;
	left = 0;
	right = 1;
	system("xdotool search --name DOSBox keyup Left");
	system("xdotool search --name DOSBox keydown Right");
      }
      if(curx > (initx + 50))
      {
	sideflag = 1;
	right = 0;
	left = 1;
	system("xdotool search --name DOSBox keyup Right");
	system("xdotool search --name DOSBox keydown Left");
      }
      
      
      if((curheight > (initheight + 75)) || (curwidth > (initwidth + 75)))
      {
	upflag = 1;
	down = 0;
	up = 1;
	system("xdotool search --name DOSBox keyup Down");
	system("xdotool search --name DOSBox keydown Up");
      }
      if((curwidth < (initwidth - 50)) || (curheight < (initheight - 50)))
      {
	upflag = 1;
	up = 0;
	down = 1;
	system("xdotool search --name DOSBox keyup Up");
	system("xdotool search --name DOSBox keydown Down");
      }
      
      cout<<curheight<<"\t"<<curwidth<<endl;
      cout<<"up = "<<up<<"\tdown = "<<down<<"\tleft = "<<left<<"\tright = "<<right<<endl;
      cout<<framecount<<endl;
continuescan:
      IplImage* bg = cvCreateImage( sz, 8, 3);
      cvRectangle( bg, cvPoint(bg->width,bg->height), cvPoint(0,0), CV_RGB( 255, 255, 255), -1, 8, 0 );
      bg->origin = 1;
      
      //Draw grid for the output window
      /*for(int b = 0; b< int(bg->width/10); b++)
      {
	cvLine( bg, cvPoint(b*20, 0), cvPoint(b*20, bg->height), CV_RGB( 200, 200, 200), 1, 8, 0 );
	cvLine( bg, cvPoint(0, b*20), cvPoint(bg->width, b*20), CV_RGB( 200, 200, 200), 1, 8, 0 );
      }
      */
      src = cvQueryFrame( capture);		//capture frame from camera input video
      framecount++;
      cvCvtColor(src, hsv_image, CV_BGR2HSV);	//convert from rgb to hsv
      cvInRangeS (hsv_image, hsv_min, hsv_max, hsv_mask);	//convert frame into binary (hsv_image -> hsv_mask
      cvSmooth( hsv_mask, hsv_mask, CV_MEDIAN, 27, 0, 0, 0 );	//use median smoothing to remove noise
      cvCanny(hsv_mask, hsv_edge, 1, 3, 5);		//Canny edge detector for edge detection
      cvFindContours( hsv_mask, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	//Find contours in the frame
      CvSeq* contours2 = NULL;
      double result = 0, result2 = 0;
      
      while(contours)		//find the contour with the largest area
      {
	result = fabs( cvContourArea( contours, CV_WHOLE_SEQ ) );	//calculate area of the contour
	if ( result > result2) {result2 = result; contours2 = contours;};
	  contours  =  contours->h_next;
      }
      
      if ( contours2 )
      {
	CvRect rect = cvBoundingRect( contours2, 0 );		//get the bounding rectangle for the largest contour
	//draw the bounding rectangle on the output window
	cvRectangle( bg, cvPoint(rect.x, rect.y + rect.height), cvPoint(rect.x + rect.width, rect.y), CV_RGB(200, 0, 200), 1, 8, 0 );
	int checkcxt = cvCheckContourConvexity( contours2 );		//check if the contour is convex
	CvSeq* hull = cvConvexHull2( contours2, 0, CV_CLOCKWISE, 0 );	//draw a convex hull around the contour
	CvSeq* defect = cvConvexityDefects( contours2, hull, dftStorage );	//return convexity defects in the contour
	
	/*
	if( defect->total >=40 ) 
	  cout << " Closed Palm " << endl;
	else if( defect->total >=30 && defect->total <40 ) 
	  cout << " Open Palm " << endl;
	else
	cout << " Fist " << endl;
	*/
	
	CvBox2D box = cvMinAreaRect2( contours2, minStorage );	//circumscribe a rectangle around the contour
	prevx = curx;
	prevy = cury;
	prevheight = curheight;
	prevwidth = curwidth;
	curx = box.center.x;
	cury = box.center.y;
	curheight = box.size.height;
	curwidth = box.size.width;
	cvCircle( bg, cvPoint(prevx, prevy), 3, CV_RGB(200, 0, 200), 2, 8, 0 );	//draw the centre of the rectangle
	cvEllipse( bg, cvPoint(prevx, prevy), cvSize(prevheight/2, prevwidth/2), box.angle, 0, 360, CV_RGB(220, 0, 220), 1, 8, 0 );				//inscribe an ellipse inside the rectangle
      }
      cvDrawContours( bg, contours2,  CV_RGB( 0, 200, 0), CV_RGB( 0, 100, 0), 1, 1, 8, cvPoint(0,0));	//draw the contour in the output window
      cvShowImage( "src", src);
      cvNamedWindow("bg",0); 
      cvShowImage("bg",bg); 
      cvReleaseImage( &bg);
      c = cvWaitKey(1);
  }

  cvReleaseCapture( &capture);
  cvDestroyAllWindows();
}

