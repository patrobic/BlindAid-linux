#pragma once

#include "IModule.h"
#include "SimulateCapture.h"

#include "DetectDepthObstacle.h"
#include "DetectTrafficLight.h"
#include "DetectStopSign.h"

class Vision : public IModule
{
public:
  class Parameters : public IParameters
  {
  public:
    DetectDepthObstacle::Parameters *GetDepthObstacleParams() { return &_dodParams; }
    DetectTrafficLight::Parameters *GetTrafficLightParams() { return &_sldParams; }
    DetectStopSign::Parameters *GetStopSignParams() { return &_ssdParams; }

  private:
    DetectDepthObstacle::Parameters _dodParams;
    DetectTrafficLight::Parameters _sldParams;
    DetectStopSign::Parameters _ssdParams;
  };

  class Results : public IResults
  {
  public:
    void Clear() {}

    DetectTrafficLight::Results *GetTrafficLightResults() { return &_tldResults; }
    DetectStopSign::Results *GetStopSignResults() { return &_ssdResults; }
    DetectDepthObstacle::Results *GetDepthObstacleResults() { return &_dodResults; }

    cv::Mat *GetCurrentColorImage() { return _currentColorImage; }
    cv::Mat *GetCurrentDepthImage() { return _currentDepthImage; }

    void SetCurrentColorImage(cv::Mat *mat) { _currentColorImage = mat; }
    void SetCurrentDepthImage(cv::Mat *mat) { _currentDepthImage = mat; }

  private:
    DetectTrafficLight::Results _tldResults;
    DetectStopSign::Results _ssdResults;
    DetectDepthObstacle::Results _dodResults;

    cv::Mat *_currentColorImage;
    cv::Mat *_currentDepthImage;
  };

  Vision(Data *data, IParameters *params, IResults *input, IResults *output);
  void operator()();

private:
  Parameters *_params;
  CaptureSim::Results *_input;
  Results *_output;

  DetectStopSign _stopSign;
  DetectTrafficLight _trafficLight;
  DetectDepthObstacle _depthObstacle;
};