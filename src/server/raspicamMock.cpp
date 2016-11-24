#include <camerasp/raspicamMock.hpp>
#include <vector>
#include <jpeg\jpgrdwr.h>
#include <camerasp/parseCmd.hpp>
namespace raspicam
{
  RaspiCam::RaspiCam(void) { console->info("RaspiCam"); }
  RaspiCam::~RaspiCam(void) { console->info("~RaspiCam"); }
  void  RaspiCam::setFormat(enum RASPICAM_FORMAT) { console->info("setFormat"); }
  void  RaspiCam::setWidth(unsigned int w) { console->info("setWidth: " "{0}" , w); width = w; }
  void  RaspiCam::setHeight(unsigned int h) { console->info("setHeight: " "{0}" , h); height = h; }
  void  RaspiCam::setBrightness(unsigned int) { console->info("setBrightness"); }
  void  RaspiCam::setSharpness(int) { console->info("setSharpness"); }
  void  RaspiCam::setContrast(int) { console->info("setContrast"); }
  void  RaspiCam::setISO(int) { console->info("setISO"); }
  void  RaspiCam::setSaturation(int) { console->info("setSaturation"); }
  void  RaspiCam::setVideoStabilization(bool) { console->info("setVideoStabilization"); }
  void  RaspiCam::setExposureCompensation(int) { console->info("setExposureCompensation"); }
  void  RaspiCam::setExposure(enum RASPICAM_EXPOSURE) { console->info("setExposure"); }
  void  RaspiCam::setShutterSpeed(unsigned int) { console->info("setShutterSpeed"); }
  void  RaspiCam::setAWB(enum RASPICAM_AWB) { console->info("setAWB"); }
  void  RaspiCam::setAWB_RB(float, float) { console->info("setAWB_RB"); }
  void  RaspiCam::startCapture() { console->info("startCapture"); }
  void  RaspiCam::stopCapture() { console->info("stopCapture"); }
  void  RaspiCam::commitParameters() { console->info("commitParameters"); }
  unsigned int  RaspiCam::getImageBufferSize(void)const { console->info("getImageBufferSize"); return 3 * this->height*this->width; }
  unsigned int  RaspiCam::getWidth(void)const { console->info("getWidth"); return width; }
  unsigned int  RaspiCam::getHeight(void)const { console->info("getHeight"); return height; }
  void RaspiCam::retrieve(unsigned char *) { console->info("retrieve"); }
  bool RaspiCam::grab(void) { console->info("grab"); return true; }
  bool RaspiCam::open(bool) { console->info("open"); return true; }

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