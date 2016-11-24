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
#include <cstdarg>


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
        console->info("{0} : {1}", param, n);
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

namespace Camerasp
{
  void processCommandLine(std::map<std::string, std::string> const& nameVal, raspicam::RaspiCam &Camera) {
    Camera.setWidth(getParamVal("width", nameVal, 640)); //max 1280
    Camera.setHeight(getParamVal("height", nameVal, 480)); //max 960
    Camera.setBrightness(getParamVal("brightness", nameVal, 50));

    Camera.setSharpness(getParamVal("sharpness", nameVal, 0));
    Camera.setContrast(getParamVal("contrast", nameVal, 0));
    Camera.setSaturation(getParamVal("saturation", nameVal, 0));
    Camera.setShutterSpeed(getParamVal("shutterspeed", nameVal, 0));
    Camera.setISO(getParamVal("iso", nameVal, 400));
    Camera.setVideoStabilization(getParamVal("videostabilisation", nameVal, false));
    Camera.setExposureCompensation(getParamVal("ec", nameVal, 0));

    auto it = nameVal.find("format");
    if (it != end(nameVal))
      Camera.setFormat(getFormatFromString(it->second));
    it = nameVal.find("exposure");
    if (it != end(nameVal))
      Camera.setExposure(getExposureFromString(it->second));

    it = nameVal.find("awb");
    if (it != end(nameVal))
      Camera.setAWB(getAwbFromString(it->second));
    //To Do: unimplemented feature
    //Camera.setAWB_RB(float(getParamVal("awb_b",args ,1)/100.0), float(getParamVal("awb_g",args ,1)/100.0));


  }
  
    std::string lowerCase(std::string const& str)
    {
      std::string retval(str);
      std::transform(std::begin(str), std::end(str), std::begin(retval), tolower);
      return retval;
    }

    std::map<std::string, std::string>
      tokenize(std::string const& query)
    {
      using namespace std;
      map<string, string> result;
      size_t offset = 0;
      size_t pos = query.find('=', offset);
      while (pos != string::npos)
      {
        string name = query.substr(offset, pos - offset);
        offset = pos + 1;
        pos = query.find('&', offset);
        if (pos != string::npos && pos > offset)
        {
          string val = query.substr(offset, pos - offset);
          result.insert(make_pair(lowerCase(name), val));
          offset = pos + 1;
        }
        else {
          string val = query.substr(offset);
          if (!val.empty())
            result.insert(make_pair(lowerCase(name), val));
          break;
        }
        pos = query.find('=', offset);
      }
      return result;
    }
    errno_t readOptions(std::string const& fileName, std::string& options)
    {
      FILE *fp = nullptr;
      errno_t err = fopen_s(&fp, fileName.c_str(), "r");
      if (err == 0)
      {
        for (int c = getc(fp); c != EOF; c = getc(fp))
        {
          if (!isspace(c)) options.push_back(c);
        }
        fclose(fp);
      }
      return err;
    }

}
