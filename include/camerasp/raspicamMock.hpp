#ifndef CAMERASP_RASPICAM_MOCK
#define CAMERASP_RASPICAM_MOCK
#include <raspicam/raspicamtypes.h>
namespace raspicam
{
	class RaspiCam
	{
	public:
		void  setFormat(enum RASPICAM_FORMAT){}
		void  setWidth(unsigned int w){ width=w; }
		void  setHeight(unsigned int h){ height = h;}
		void  setBrightness(unsigned int){}
		void  setSharpness(int){}
		void  setContrast(int){}
		void  setISO(int){}
		void  setSaturation(int){}
		void  setVideoStabilization(bool){}
		void  setExposureCompensation(int){}
		void  setExposure(enum RASPICAM_EXPOSURE){}
		void  setShutterSpeed(unsigned int){}
		void  setAWB(enum RASPICAM_AWB){}
		void  setAWB_RB(float,float){}
		RaspiCam(void){}
		~RaspiCam(void){}
    bool  open(bool val = true) { return true; }
    bool  grab(void) { return true; }
		void  retrieve(unsigned char *){}
		unsigned int  getImageBufferSize(void)const {return width*height;}
		unsigned int  getWidth(void)const {return width;}
    unsigned int  getHeight(void)const { return height; }
	private:
		unsigned int width;
		unsigned int height;
  };
}
#endif