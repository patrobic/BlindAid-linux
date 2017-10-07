#include "DetectTrafficLight.h"

using namespace std;
using namespace cv;

void DetectTrafficLight::operator()()
{
  Clear();
  Process();
}

void DetectTrafficLight::Process()
{
  PreProcess();

  switch (_params->GetMode())
  {
  case Parameters::BlobDetectorMode:
    BlobDetectorApproach();
    break;
  case Parameters::HoughCirclesMode:
    HoughCirclesApproach();
    break;
  case Parameters::FindContoursMode:
    FindCountoursApproach();
    break;
  }

  DetectRectangle();
}

void DetectTrafficLight::PreProcess()
{
  cvtColor(*_input->GetColorImage(), _hsvImage, CV_BGR2HSV);

  split(*_input->GetColorImage(), _bgrChannels);
  split(_hsvImage, _hsvChannels);
 
  Mat redRegionUpper;
  Mat redRegionLower;
  inRange(_hsvImage, cv::Scalar(150, 150, 180), cv::Scalar(180, 255, 255), redRegionUpper);
  inRange(_hsvImage, cv::Scalar(0, 150, 180), cv::Scalar(10, 255, 255), redRegionLower);
  _rMask = redRegionUpper + redRegionLower;

  //threshold(_h, _hMask, 170, 255, THRESH_TOZERO);
  //threshold(_hMask, _hMask, 190, 255, THRESH_TOZERO_INV);
  //dilate(_rMask, _rMask, Mat(), Point(-1, -1), 1);
}

void DetectTrafficLight::FindCountoursApproach()
{
  vector<vector<cv::Point>> contours;
  vector<Vec4i> hierarchy;
  double circleArea;
  double rectArea;
  vector<double> score;
  float circularityThreshold = 0.5;

  findContours(_rMask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
  
  vector<Point2f>center(contours.size());
  vector<float>radius(contours.size());

  for (int i = 0; i < contours.size(); i++)
  {
    drawContours(*_input->GetColorImage(), contours, i, 255, 1, 8, vector<Vec4i>(), 0, Point());

    circleArea = contourArea(contours[i]);
    Rect rect = boundingRect(Mat(contours[i]));
    rectArea = CV_PI * ((rect.width + rect.height) ^ 2 / 4);

    score.push_back(1 - abs(circleArea - rectArea) / rectArea);

    if (score.at(i) > circularityThreshold)
    {
      minEnclosingCircle((Mat)contours[i], center[i], radius[i]);
      drawContours(*_input->GetColorImage(), contours, i, 255, 1, 8, vector<Vec4i>(), 0, Point());
     
      _output->PushBack(Circle(center[i], (int)radius[i]));
    }
  }
}

void DetectTrafficLight::HoughCirclesApproach()
{
  vector<vector<cv::Point>> contours;
  vector<Vec4i> hierarchy;
  vector<cv::Vec3f> circles;

  // Option 1: Trace circle outline on mask using FindContours
  //findContours(_rMask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

  //_rMask = Scalar::all(0);

  //for(int i = 0; i<contours.size(); ++i)
  //  drawContours(_rMask, contours, i, 255, 1, 8, vector<Vec4i>(), 0, Point());

  // Option 2: Get circle outlines by using edge detector
  int th = 100;
  Canny(_rMask, _rMask, th, th * 3);

  HoughCircles(_rMask, circles, HOUGH_GRADIENT, 1, _rMask.cols / 20, 100, 12, 1, 50);

  for (size_t i = 0; i < circles.size(); i++)
    _output->PushBack(Circle(Point(cvRound(circles[i][0]), cvRound(circles[i][1])), (int)cvRound(circles[i][2])));
}

void DetectTrafficLight::BlobDetectorApproach()
{
  SimpleBlobDetector::Params params;

  params.filterByArea = true;
  params.minArea = 4 * 4;
  params.maxArea = 60 * 60;
  params.filterByCircularity = true;
  params.minCircularity = 0.1f;
  params.filterByConvexity = true;
  params.minConvexity = 0.8f;
  params.filterByInertia = true;
  params.minInertiaRatio = 0.5f;
  params.filterByColor = true;
  params.blobColor = 255;

  Ptr<SimpleBlobDetector> sbd = SimpleBlobDetector::create(params);

  vector<KeyPoint> keypoints;
  sbd->detect(_rMask, keypoints);
  
  for (int i = 0; i < keypoints.size(); i++)
    _output->PushBack(Circle(keypoints[i].pt, (int)keypoints[i].size));
}

void DetectTrafficLight::DetectRectangle()
{
  int sizeFactor = 12;
  Mat box;
  Rect rect;
  for (int i = 0; i < _output->Size(); ++i)
  {
    rect.x = std::max(0, _output->At(i)._center.x - _output->Size() * sizeFactor);
    rect.y = std::max(0, _output->At(i)._center.y - _output->Size() * sizeFactor);
    rect.width = std::min(_input->GetColorImage()->cols - rect.x - 1, _output->Size() * 2 * sizeFactor);
    rect.height= std::min(_input->GetColorImage()->rows - rect.y - 1, _output->Size() * 2 * sizeFactor);

    box = _v(rect);

    threshold(box, box, 70, 255, THRESH_BINARY_INV);
    dilate(box, box, Mat(), Point(-1, -1), 1);
    erode(box, box, Mat(), Point(-1, -1), 2);

    vector<vector<cv::Point>> contours;
    vector<Vec4i> hierarchy;

    findContours(box, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

    vector<vector<Point> > contours_poly;
    contours_poly.resize(contours.size());


    for (int j = 0; j < contours_poly.size(); ++j)
    {
      approxPolyDP(Mat(contours[j]), contours_poly[j], 10, true);
      drawContours(box, contours_poly, j, 128, 1, 8, vector<Vec4i>(), 0, Point());
    }
    // TODO
    // detect if shape is near rectangular.
    // shape contains original light region.
  }
}

void DetectTrafficLight::Clear()
{
  _output->Clear();
}