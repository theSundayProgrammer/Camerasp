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

//parse command line
//returns the value of a command line param. If not found, defvalue is returned
bool getParamVal ( std::string param,std::vector<std::string> const & args, bool defvalue ) {
    using namespace std;
    auto it = find(begin(args),end(args), param) ;
    if (it == end(args) || (it+1) ==  end(args)) 
        return defvalue;
    else
    {
        std::string str(*++it);
        return str== string("true");
    }
}

int getParamVal ( std::string param,std::vector<std::string> const & args, int defvalue ) {
    using namespace std;
    auto it = find(begin(args),end(args), param) ;
    if (it == end(args) || (it+1) ==  end(args)) 
        return defvalue;
    else
    {
        int n = atoi((*++it).c_str());
        return n;
    }
}

std::vector<std::string>::const_iterator 
findParam(std::string const& param,std::vector<std::string> const & args)
{
   using namespace std;
   return std::find(begin(args),end(args),param);
}
raspicam::RASPICAM_EXPOSURE getExposureFromString ( std::string str ) {
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

raspicam::RASPICAM_AWB getAwbFromString ( std::string str ) {
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
raspicam::RASPICAM_FORMAT getFormatFromString ( std::string str ) {
    if(str=="GREY") return raspicam::RASPICAM_FORMAT_GRAY;
    if(str=="YUV") return raspicam::RASPICAM_FORMAT_YUV420;
    return raspicam::RASPICAM_FORMAT_RGB;

}
void processCommandLine ( std::vector<std::string> const & args,raspicam::RaspiCam &Camera ) {
    Camera.setWidth ( getParamVal ( "w",args,640 ) ); //max 1280
    Camera.setHeight ( getParamVal ( "h",args,480 ) ); //max 960
    Camera.setBrightness ( getParamVal ( "br",args,50 ) );

    Camera.setSharpness ( getParamVal ( "sh",args,0 ) );
    Camera.setContrast ( getParamVal ( "co",args,0 ) );
    Camera.setSaturation ( getParamVal ( "sa",args,0 ) );
    Camera.setShutterSpeed( getParamVal ( "ss",args,0 ) );
    Camera.setISO ( getParamVal ( "iso",args ,400 ) );
    Camera.setVideoStabilization ( getParamVal ( "vs",args,false ) );
    Camera.setExposureCompensation ( getParamVal ( "ec",args ,0 ) );
    
    auto idx=findParam ( "format",args );
    if ( idx != args.end() )
      Camera.setFormat(getFormatFromString(*++idx));
    idx=findParam ( "ex",args );
    if ( idx != args.end() )
        Camera.setExposure ( getExposureFromString ( *++idx ) );
    idx=findParam ( "awb",args ) ;
    if ( idx != args.end() )
        Camera.setAWB( getAwbFromString ( *++idx ) );
    Camera.setAWB_RB(float(getParamVal("awb_b",args ,1)/100.0), float(getParamVal("awb_g",args ,1)/100.0));

}

