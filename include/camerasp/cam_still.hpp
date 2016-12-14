/**********************************************************
 Software developed by AVA ( Ava Group of the University of Cordoba, ava  at uco dot es)
 Main author Rafael Munoz Salinas (rmsalinas at uco dot es)
 This software is released under BSD license as expressed below
-------------------------------------------------------------------
Copyright (c) 2013, AVA ( Ava Group University of Cordoba, ava  at uco dot es)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:

   This product includes software developed by the Ava group of the University of Cordoba.

4. Neither the name of the University nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY AVA ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL AVA BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************/
#pragma once
#ifndef CAMERASP_CAM_STILL_HPP
#define CAMERASP_CAM_STILL_HPP
#include "raspicamtypes.h"
#include <string>
#include <atomic>
#include <mmal/mmal.h>
struct MMAL_COMPONENT_T;
struct MMAL_CONNECTION_T;
struct MMAL_POOL_T;
struct MMAL_PORT_T;
#define MMAL_CAMERA_CAPTURE_PORT 2
#define STILLS_FRAME_RATE_NUM 3
#define STILLS_FRAME_RATE_DEN 1
namespace camerasp {
  typedef void(*imageTakenCallback) (unsigned char * data, int error, unsigned int length);

  class cam_still {

  private:

    MMAL_COMPONENT_T * camera;	 /// Pointer to the camera component
    MMAL_COMPONENT_T * encoder;	/// Pointer to the encoder component
    MMAL_CONNECTION_T * encoder_connection; // Connection from the camera to the encoder
    MMAL_POOL_T * encoder_pool;				  /// Pointer to the pool of buffers used by encoder output port
    MMAL_PORT_T * camera_still_port;
    MMAL_PORT_T * encoder_input_port;
    MMAL_PORT_T * encoder_output_port;
    unsigned int width;
    unsigned int height;
    unsigned int rotation; // 0 to 359
    unsigned int brightness; // 0 to 100
    unsigned int quality; // 0 to 100
    int iso;
    int sharpness; // -100 to 100
    int contrast; // -100 to 100
    int saturation; // -100 to 100
    raspicam::RASPICAM_ENCODING encoding;
    raspicam::RASPICAM_EXPOSURE exposure;
    raspicam::RASPICAM_AWB awb;
    raspicam::RASPICAM_IMAGE_EFFECT imageEffect;
    raspicam::RASPICAM_METERING metering;
    bool changedSettings;
    bool horizontalFlip;
    bool verticalFlip;

    MMAL_FOURCC_T convertEncoding(raspicam::RASPICAM_ENCODING encoding);
    MMAL_PARAM_EXPOSUREMETERINGMODE_T convertMetering(raspicam::RASPICAM_METERING metering);
    MMAL_PARAM_EXPOSUREMODE_T convertExposure(raspicam::RASPICAM_EXPOSURE exposure);
    MMAL_PARAM_AWBMODE_T convertAWB(raspicam::RASPICAM_AWB awb);
    MMAL_PARAM_IMAGEFX_T convertImageEffect(raspicam::RASPICAM_IMAGE_EFFECT imageEffect);
    void commitBrightness();
    void commitQuality();
    void commitRotation();
    void commitISO();
    void commitSharpness();
    void commitContrast();
    void commitSaturation();
    void commitExposure();
    void commitAWB();
    void commitImageEffect();
    void commitMetering();
    void commitFlips();
    int startCapture();
    int createCamera();
    int createEncoder();
    void destroyCamera();
    void destroyEncoder();
    void setDefaults();
    MMAL_STATUS_T connectPorts(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection);
    void setEncoding(raspicam::RASPICAM_ENCODING encoding);

    bool _isInitialized;




  public:
    ~cam_still();
    cam_still();
    int initialize();
    bool open();
    //Grabs and set the data into the data buffer which has the indicated length. It is your responsability
    // to alloc the buffer. You can use getImageBufferSize for that matter.
    //bool grab_retrieve ( unsigned char * data, unsigned int length );
    //int startCapture(imageTakenCallback userCallback, unsigned char * preallocated_data, unsigned int offset, unsigned int length);
    void stopCapture();
    int takePicture(unsigned char * preallocated_data, unsigned int length);
    void release();
    size_t getImageBufferSize() const;
    //void bufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    void commitParameters();
    void setWidth(unsigned int width);
    void setHeight(unsigned int height);
    void setCaptureSize(unsigned int width, unsigned int height);
    void setBrightness(unsigned int brightness);
    void setQuality(unsigned int quality);
    void setRotation(int rotation);
    void setISO(int iso);
    void setSharpness(int sharpness);
    void setContrast(int contrast);
    void setSaturation(int saturation);
    void setExposure(raspicam::RASPICAM_EXPOSURE exposure);
    void setAWB(raspicam::RASPICAM_AWB awb);
    void setImageEffect(raspicam::RASPICAM_IMAGE_EFFECT imageEffect);
    void setMetering(raspicam::RASPICAM_METERING metering);
    void setHorizontalFlip(bool hFlip);
    void setVerticalFlip(bool vFlip);

    unsigned int getWidth();
    unsigned int getHeight();
    unsigned int getBrightness();
    unsigned int getRotation();
    unsigned int getQuality();
    int getISO();
    int getSharpness();
    int getContrast();
    int getSaturation();
    raspicam::RASPICAM_ENCODING getEncoding();
    raspicam::RASPICAM_EXPOSURE getExposure();
    raspicam::RASPICAM_AWB getAWB();
    raspicam::RASPICAM_IMAGE_EFFECT getImageEffect();
    raspicam::RASPICAM_METERING getMetering();
    bool isHorizontallyFlipped();
    bool isVerticallyFlipped();
    std::atomic_flag captured;

    //Returns an id of the camera. We assume the camera id is the one of the raspberry
    //the id is obtained using raspberry serial number obtained in /proc/cpuinfo
    static std::string getId();

  };

}
#endif // raspicam::RASPICAM_H
