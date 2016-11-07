//////////////////////////////////////////////////////////////////////////////
// Copyright (c) Joseph Mariadassou
// theSundayProgrammer@gmail.com
// adapted from Raspicam sample code
// Distributed under the Boost Software License, Version 1.0.
// 
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <algorithm>
#include <camerasp/parseCmd.hpp>
#include <map>
#include <iostream>
//parse command line
//returns the value of a command line param. If not found, defvalue is returned
bool getParamVal ( std::string param,std::map<std::string,std::string> const& args, bool defvalue ) {
    using namespace std;
    auto it = args.find(param);
    if (it == end(args)) 
        return defvalue;
    else
    {
       
        return it->second == string("true");
    }
}

int getParamVal ( std::string param,std::map<std::string,std::string> const& args, int defvalue ) {
    using namespace std;
    using namespace std;
    auto it = args.find(param);
    if (it == end(args))
      return defvalue;
    else
    {
        int n = atoi(it->second.c_str());
        std::cout << param << ":" << n << std::endl;
        return n;
    }
}


raspicam::RASPICAM_EXPOSURE getExposureFromString (const  std::string& str ) {
    if ( str=="OFF" ) return raspicam::RASPICAM_EXPOSURE_OFF;
    if ( str=="AUTO" ) return raspicam::RASPICAM_EXPOSURE_AUTO;
    if ( str=="NIGHT" ) return raspicam::RASPICAM_EXPOSURE_NIGHT;
    if ( str=="NIGHTPREVIEW" ) return raspicam::RASPICAM_EXPOSURE_NIGHTPREVIEW;
    if ( str=="BACKLIGHT" ) return raspicam::RASPICAM_EXPOSURE_BACKLIGHT;
    if ( str=="SPOTLIGHT" ) return raspicam::RASPICAM_EXPOSURE_SPOTLIGHT;
    if ( str=="SPORTS" ) return raspicam::RASPICAM_EXPOSURE_SPORTS;
    if ( str=="SNOW" ) return raspicam::RASPICAM_EXPOSURE_SNOW;
    if ( str=="BEACH" ) return raspicam::RASPICAM_EXPOSURE_BEACH;
    if ( str=="VERYLONG" ) return raspicam::RASPICAM_EXPOSURE_VERYLONG;
    if ( str=="FIXEDFPS" ) return raspicam::RASPICAM_EXPOSURE_FIXEDFPS;
    if ( str=="ANTISHAKE" ) return raspicam::RASPICAM_EXPOSURE_ANTISHAKE;
    if ( str=="FIREWORKS" ) return raspicam::RASPICAM_EXPOSURE_FIREWORKS;
    return raspicam::RASPICAM_EXPOSURE_AUTO;
}

raspicam::RASPICAM_AWB getAwbFromString ( const std::string& str ) {
if ( str=="OFF" ) return raspicam::RASPICAM_AWB_OFF;
if ( str=="AUTO" ) return raspicam::RASPICAM_AWB_AUTO;
if ( str=="SUNLIGHT" ) return raspicam::RASPICAM_AWB_SUNLIGHT;
if ( str=="CLOUDY" ) return raspicam::RASPICAM_AWB_CLOUDY;
if ( str=="SHADE" ) return raspicam::RASPICAM_AWB_SHADE;
if ( str=="TUNGSTEN" ) return raspicam::RASPICAM_AWB_TUNGSTEN;
if ( str=="FLUORESCENT" ) return raspicam::RASPICAM_AWB_FLUORESCENT;
if ( str=="INCANDESCENT" ) return raspicam::RASPICAM_AWB_INCANDESCENT;
if ( str=="FLASH" ) return raspicam::RASPICAM_AWB_FLASH;
if ( str=="HORIZON" ) return raspicam::RASPICAM_AWB_HORIZON;
return raspicam::RASPICAM_AWB_AUTO;
}
raspicam::RASPICAM_FORMAT getFormatFromString ( const std::string& str ) {
    if(str=="GREY") return raspicam::RASPICAM_FORMAT_GRAY;
    if(str=="YUV") return raspicam::RASPICAM_FORMAT_YUV420;
    return raspicam::RASPICAM_FORMAT_RGB;

}
void processCommandLine (std::map<std::string, std::string> const& nameVal,raspicam::RaspiCam &Camera ) {
    Camera.setWidth ( getParamVal ( "width",nameVal,640 ) ); //max 1280
    Camera.setHeight ( getParamVal ( "height",nameVal,480 ) ); //max 960
    Camera.setBrightness ( getParamVal ( "brightness",nameVal,50 ) );

    Camera.setSharpness ( getParamVal ( "sharpness",nameVal,0 ) );
    Camera.setContrast ( getParamVal ( "contrast",nameVal,0 ) );
    Camera.setSaturation ( getParamVal ( "saturation",nameVal,0 ) );
    Camera.setShutterSpeed( getParamVal ( "shutterspeed",nameVal,0 ) );
    Camera.setISO ( getParamVal ( "iso",nameVal ,400 ) );
    Camera.setVideoStabilization ( getParamVal ( "videostabilisation",nameVal,false ) );
    Camera.setExposureCompensation ( getParamVal ( "ec",nameVal ,0 ) );
    
    auto it = nameVal.find("format");
    if (it != end(nameVal))
      Camera.setFormat(getFormatFromString(it->second));
    it = nameVal.find("exposure");
    if (it != end(nameVal))
      Camera.setExposure(getExposureFromString(it->second));

    it = nameVal.find("awb");
    if (it != end(nameVal))
        Camera.setAWB( getAwbFromString ( it->second ) );
    //To Do: unimplemented feature
    //Camera.setAWB_RB(float(getParamVal("awb_b",args ,1)/100.0), float(getParamVal("awb_g",args ,1)/100.0));


}

