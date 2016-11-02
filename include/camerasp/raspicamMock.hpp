#ifndef CAMERASP_RASPICAM_MOCK
#define CAMERASP_RASPICAM_MOCK
#include <raspicam/raspicamtypes.h>

namespace raspicam
{
	class RaspiCam
	{
	public:
		RaspiCam(void);
		~RaspiCam(void);
    void  setFormat(enum RASPICAM_FORMAT) ;
    void  setWidth(unsigned int w) ;
    void  setHeight(unsigned int h) ;
    void  setBrightness(unsigned int) ;
    void  setSharpness(int) ;
    void  setContrast(int) ;
    void  setISO(int) ;
    void  setSaturation(int) ;
    void  setVideoStabilization(bool) ;
    void  setExposureCompensation(int) ;
    void  setExposure(enum RASPICAM_EXPOSURE) ;
    void  setShutterSpeed(unsigned int) ;
    void  setAWB(enum RASPICAM_AWB) ;
    void  setAWB_RB(float, float) ;
    void startCapture() ;
    void stopCapture() ;
    void commitParameters() ;
    bool  open(bool val = true) ;
    bool  grab(void) ;
		void  retrieve(unsigned char *);
		unsigned int  getImageBufferSize(void)const ;
		unsigned int  getWidth(void)const ;
    unsigned int  getHeight(void)const ;
	private:
		unsigned int width;
		unsigned int height;
  };
}
#endif