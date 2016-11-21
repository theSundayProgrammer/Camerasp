#include <camerasp/raspicamMock.hpp>
#include <iostream>
#include <vector>
#include <jpeg\jpgrdwr.h>
namespace raspicam
{
  RaspiCam::RaspiCam(void) { std::cout << "RaspiCam" << std::endl; }
  RaspiCam::~RaspiCam(void) { std::cout << "~RaspiCam" << std::endl; }
  void  RaspiCam::setFormat(enum RASPICAM_FORMAT) { std::cout << "setFormat" << std::endl; }
  void  RaspiCam::setWidth(unsigned int w) { std::cout << "setWidth: " << w << std::endl; width = w; }
  void  RaspiCam::setHeight(unsigned int h) { std::cout << "setHeight: "<< h << std::endl; height = h; }
  void  RaspiCam::setBrightness(unsigned int) { std::cout << "setBrightness" << std::endl; }
  void  RaspiCam::setSharpness(int) { std::cout << "setSharpness" << std::endl; }
  void  RaspiCam::setContrast(int) { std::cout << "setContrast" << std::endl; }
  void  RaspiCam::setISO(int) { std::cout << "setISO" << std::endl; }
  void  RaspiCam::setSaturation(int) { std::cout << "setSaturation" << std::endl; }
  void  RaspiCam::setVideoStabilization(bool) { std::cout << "setVideoStabilization" << std::endl; }
  void  RaspiCam::setExposureCompensation(int) { std::cout << "setExposureCompensation" << std::endl; }
  void  RaspiCam::setExposure(enum RASPICAM_EXPOSURE) { std::cout << "setExposure" << std::endl; }
  void  RaspiCam::setShutterSpeed(unsigned int) { std::cout << "setShutterSpeed" << std::endl; }
  void  RaspiCam::setAWB(enum RASPICAM_AWB) { std::cout << "setAWB" << std::endl; }
  void  RaspiCam::setAWB_RB(float, float) { std::cout << "setAWB_RB" << std::endl; }
  void  RaspiCam::startCapture() { std::cout << "startCapture" << std::endl; }
  void  RaspiCam::stopCapture() { std::cout << "stopCapture" << std::endl; }
  void  RaspiCam::commitParameters() { std::cout << "commitParameters" << std::endl; }
  unsigned int  RaspiCam::getImageBufferSize(void)const { std::cout << "getImageBufferSize" << std::endl; return 3 * this->height*this->width; }
  unsigned int  RaspiCam::getWidth(void)const { std::cout << "getWidth" << std::endl; return width; }
  unsigned int  RaspiCam::getHeight(void)const { std::cout << "getHeight" << std::endl; return height; }
  void RaspiCam::retrieve(unsigned char *) { std::cout << "retrieve" << std::endl; }
  bool RaspiCam::grab(void) { std::cout << "grab" << std::endl; return true; }
  bool RaspiCam::open(bool) { std::cout << "open" << std::endl; return true; }

}
namespace Camerasp
{
  std::vector<unsigned char> write_JPEG_dat(struct Camerasp::ImgInfo const &dat)
  {
    std::vector<unsigned char> buffer;
    FILE *fp = nullptr;
    fopen_s(&fp, "C:\\Users\\Public\\Pictures\\figure_3.jpg", "rb");
    if (fp)
    {
      for (int c = getc(fp); c != EOF; c = getc(fp))
      {
        buffer.push_back(c);
      }
      fclose(fp);
    }
    return buffer;

  }
}