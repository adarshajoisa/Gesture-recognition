#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>
#define centerX 270
#define centerY 210

using namespace std;

IplImage* GetThresholdedImage(IplImage* img)
{
  // Convert the image into an HSV image
  IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
  cvCvtColor(img, imgHSV, CV_BGR2HSV);

  IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);
  cvSmooth( imgHSV, imgHSV, CV_BLUR, 27, 0, 0, 0 );
  
  
  //TODO: executing xdotool using search window option is extremely slow. Figure out a way around it.
/*
 * HINT for converting from KDE HSV to OpenCV HSV: multiply the KDE value by 0.711 to get OpenCV equivalent HSV value
 * HSV VALUES FOR SOME COLORS:
 * cvScalar(88, 7, 30), cvScalar(110, 37, 40) : black. Tried with my walkman
 * 14.132.148-11.124.146 red: tried with a coke cap
 * 
 * 
 * 
 * 
 */
//   Values 20,100,100 to 30,255,255 working for yellow
  cvInRangeS(imgHSV, cvScalar(10, 124, 140), cvScalar(20, 140, 146), imgThreshed);
//   cvInRangeS(imgHSV, cvScalar(0, 30, 80), cvScalar(20, 150, 255), imgThreshed);		//Tracking hand
//   cvInRangeS(imgHSV, cvScalar(20, 100, 100), cvScalar(30, 255, 255), imgThreshed);	//Tracking PP ball

  cvReleaseImage(&imgHSV);

  return imgThreshed;
}

int main()
{
  CvCapture* capture = 0;
  capture = cvCaptureFromCAM(0);	

  int framecount = 0;
  
  if(!capture)
  {
    printf("Could not initialize capturing...\n");
    return -1;
  }
  
  
  int choice;
  cout<<"What do you want to do?"<<endl;
  cout<<"1. Control the mouse."<<endl;
  cout<<"2. Control arrow keys."<<endl;
  
  while(1)
  {
    cout<<"Enter your choice: ";
    cin>>choice;
    if( choice < 1 || choice > 2 )
    {
      cout<<"Invalid output. Try again."<<endl;
    }
    else break;
  }
  sleep(1);
  if(choice == 1)
    cout<<"Starting mouse control"<<endl;
  else if(choice == 2)
    cout<<"Starting keyboard control"<<endl;
  
  
//   cvNamedWindow("video");
//   cvNamedWindow("thresh");
  
  int left = 0, right = 0, up = 0, down = 0;
  char ch;
  
  while(true)
  {
    IplImage* frame = 0;
    frame = cvQueryFrame(capture);
    framecount++;

    if(!frame)
      break;
    

    // Holds the thresholded image (red = white, rest = black)
    IplImage* imgThresh = GetThresholdedImage(frame);

    // Calculate the moments to estimate the position of the ball
    CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
    cvMoments(imgThresh, moments, 1);

    // The actual moment values
    double moment10 = cvGetSpatialMoment(moments, 1, 0);
    double moment01 = cvGetSpatialMoment(moments, 0, 1);
    double area = cvGetCentralMoment(moments, 0, 0);

    // Holding the last and current ball positions
    static int posX = 100;
    static int posY = 100;

    int lastX = posX;
    int lastY = posY;

    posX = moment10/area;
    posY = moment01/area;
    
    if(posX <= 0)
      posX = lastX;
    if(posY <= 0)
      posY = lastY;
    //invert the sideways movement to get a mirror image of movement
    posX = 640 - posX;		//the camera doesn't give a mirror image. We need a mirror image to get the left-right directions correct.

//     printf("position (%d,%d)\n", posX, posY);
    char command[50], clickcmd[50];
//     ch = cvWaitKey(1);
//     if(ch == ' ')
//       strcpy(clickcmd,"xdotool mousedown 1");
//     else
//       strcpy(clickcmd,"xdotool mouseup 1");
    sprintf(command, "xdotool mousemove %d %d", (int)posX*2, (int) (posY*1.66));
    if(choice == 1)
    {
//       system(clickcmd);
      system(command);
    }
    else
    {
      int count = 0;
      //xdotool runs very slow here when it has to search for a window. Try speeding it up
      
      //IMP: just found that the search function is the culprit, as suspected. if no window is specified, the key goes to the currently active window, and it works perfectly fine for any window! Yay!!! :) Now we need to figure out a way to get focus on a particular window, and PROBLEM SOLVED!!! :)
      left = 0, right = 0, up = 0, down = 0;
      if(posX > 250 && posX < 390 )
      {
	count ++;
	left = right = 0;
	system("xdotool keyup Left");
	system("xdotool keyup Down");
      }
      else if( posX < 250 )
      {
	count ++;
	left = 1;
	system("xdotool keyup Right");
	system("xdotool keydown Left");
      }
      else if( posX > 390 )
      {
	count ++;
	right = 1;
	system("xdotool keyup Left");
	system("xdotool keydown Right");
      }
      
      
      if( posY > 190 && posY < 270 )
      {
	count ++;
	up = down = 0;
	system("xdotool keyup Down");
	system("xdotool keyup Up");
      }
      else if( posY < 190 )
      {
	count ++;
	up = 1;
	system("xdotool keyup Down");
	system("xdotool keydown Up");
      }
      else if( posY > 270 )
      {
	count ++;
	down = 1;
	system("xdotool keyup Up");
	system("xdotool keydown Down");
      }
      
//       cout<<"count = "<<count<<endl;
      cout<<"up = "<<up<<" down = "<<down<<" left = "<<left<<" right = "<<right<<" frame = "<<framecount<<endl;
    }
    
    /*
     * Center of the screen is somewhere around (270,210). Now the movement can be controlled by comparing current position with the approximate center value.
     */

    //Invert the positions again to get the rectangle around the cap in the output video
    posX = 640 - posX;

    cvRectangle(frame, cvPoint(posX + 10, posY - 10), cvPoint(posX - 10, posY + 10), cvScalar(0, 255, 255));
//     cvAdd(frame, imgScribble, frame);
//     cvShowImage("thresh", imgThresh);
//     cvShowImage("video", frame);

    int c = cvWaitKey(10);
//     cout<<c<<endl;
    if(c!=-1)
    {
      if( c == 27 || c == 1048603)
	break;
    }

    cvReleaseImage(&imgThresh);
    delete moments;

  }
  system("clear");
  cvReleaseCapture(&capture);
  return 0;
}