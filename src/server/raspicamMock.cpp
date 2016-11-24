#include <camerasp/raspicamMock.hpp>
#include <vector>
#include <jpeg\jpgrdwr.h>
#include <camerasp/parseCmd.hpp>
namespace raspicam
{
  RaspiCam::RaspiCam(void) { console->debug("RaspiCam"); }
  RaspiCam::~RaspiCam(void) { console->debug("~RaspiCam"); }
  void  RaspiCam::setFormat(enum RASPICAM_FORMAT) { console->debug("setFormat"); }
  void  RaspiCam::setWidth(unsigned int w) { console->debug("setWidth: " "{0}" , w); width = w; }
  void  RaspiCam::setHeight(unsigned int h) { console->debug("setHeight: " "{0}" , h); height = h; }
  void  RaspiCam::setBrightness(unsigned int) { console->debug("setBrightness"); }
  void  RaspiCam::setSharpness(int) { console->debug("setSharpness"); }
  void  RaspiCam::setContrast(int) { console->debug("setContrast"); }
  void  RaspiCam::setISO(int) { console->debug("setISO"); }
  void  RaspiCam::setSaturation(int) { console->debug("setSaturation"); }
  void  RaspiCam::setVideoStabilization(bool) { console->debug("setVideoStabilization"); }
  void  RaspiCam::setExposureCompensation(int) { console->debug("setExposureCompensation"); }
  void  RaspiCam::setExposure(enum RASPICAM_EXPOSURE) { console->debug("setExposure"); }
  void  RaspiCam::setShutterSpeed(unsigned int) { console->debug("setShutterSpeed"); }
  void  RaspiCam::setAWB(enum RASPICAM_AWB) { console->debug("setAWB"); }
  void  RaspiCam::setAWB_RB(float, float) { console->debug("setAWB_RB"); }
  void  RaspiCam::startCapture() { console->debug("startCapture"); }
  void  RaspiCam::stopCapture() { console->debug("stopCapture"); }
  void  RaspiCam::commitParameters() { console->debug("commitParameters"); }
  unsigned int  RaspiCam::getImageBufferSize(void)const { console->debug("getImageBufferSize"); return 3 * this->height*this->width; }
  unsigned int  RaspiCam::getWidth(void)const { console->debug("getWidth"); return width; }
  unsigned int  RaspiCam::getHeight(void)const { console->debug("getHeight"); return height; }
  void RaspiCam::retrieve(unsigned char *) { console->debug("retrieve"); }
  bool RaspiCam::grab(void) { console->debug("grab"); return true; }
  bool RaspiCam::open(bool) { console->debug("open"); return true; }

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