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

#include <fstream>
//returns the value of a command line param. If not found, defvalue is returned
bool getParamVal ( std::string param,std::map<std::string,std::string> const& args, bool defvalue ) {
    using namespace std;
    auto it = args.find(param);
    if (it == end(args)) 
        return defvalue;
    else    {
       
        return it->second == string("true");
    }
}

int getParamVal(std::string param, std::map<std::string, std::string> const& args, int defvalue) {
  using namespace std;
  auto it = args.find(param);
  if (it == end(args))
    return defvalue;
  else {
    int n = atoi(it->second.c_str());
    console->info("{0} : {1}", param, n);
    return n;
  }
}

int getParamVal(std::string param, Json::Value const& item, int defvalue) {
  Json::Value nnType = item[param];
  if (!nnType.empty())
  {
    int n = nnType.asInt();
    console->info("{0}={1}", param, n);
    return n;
  }
  else  
    return  defvalue;
  }
namespace camerasp
{

RASPICAM_EXPOSURE getExposureFromString (const  std::string& str ) {
    if ( str=="OFF" ) return RASPICAM_EXPOSURE_OFF;
    if ( str=="AUTO" ) return RASPICAM_EXPOSURE_AUTO;
    if ( str=="NIGHT" ) return RASPICAM_EXPOSURE_NIGHT;
    if ( str=="NIGHTPREVIEW" ) return RASPICAM_EXPOSURE_NIGHTPREVIEW;
    if ( str=="BACKLIGHT" ) return RASPICAM_EXPOSURE_BACKLIGHT;
    if ( str=="SPOTLIGHT" ) return RASPICAM_EXPOSURE_SPOTLIGHT;
    if ( str=="SPORTS" ) return RASPICAM_EXPOSURE_SPORTS;
    if ( str=="SNOW" ) return RASPICAM_EXPOSURE_SNOW;
    if ( str=="BEACH" ) return RASPICAM_EXPOSURE_BEACH;
    if ( str=="VERYLONG" ) return RASPICAM_EXPOSURE_VERYLONG;
    if ( str=="FIXEDFPS" ) return RASPICAM_EXPOSURE_FIXEDFPS;
    if ( str=="ANTISHAKE" ) return RASPICAM_EXPOSURE_ANTISHAKE;
    if ( str=="FIREWORKS" ) return RASPICAM_EXPOSURE_FIREWORKS;
    return RASPICAM_EXPOSURE_AUTO;
}

RASPICAM_AWB getAwbFromString ( const std::string& str ) {
if ( str=="OFF" ) return RASPICAM_AWB_OFF;
if ( str=="AUTO" ) return RASPICAM_AWB_AUTO;
if ( str=="SUNLIGHT" ) return RASPICAM_AWB_SUNLIGHT;
if ( str=="CLOUDY" ) return RASPICAM_AWB_CLOUDY;
if ( str=="SHADE" ) return RASPICAM_AWB_SHADE;
if ( str=="TUNGSTEN" ) return RASPICAM_AWB_TUNGSTEN;
if ( str=="FLUORESCENT" ) return RASPICAM_AWB_FLUORESCENT;
if ( str=="INCANDESCENT" ) return RASPICAM_AWB_INCANDESCENT;
if ( str=="FLASH" ) return RASPICAM_AWB_FLASH;
if ( str=="HORIZON" ) return RASPICAM_AWB_HORIZON;
return RASPICAM_AWB_AUTO;
}
RASPICAM_FORMAT getFormatFromString ( const std::string& str ) {
    if(str=="GREY") return RASPICAM_FORMAT_GRAY;
    if(str=="YUV") return RASPICAM_FORMAT_YUV420;
    return RASPICAM_FORMAT_RGB;

}


  void processCommandLine(std::map<std::string, std::string> const& nameVal, camerasp::cam_still &Camera) {
    Camera.setWidth(getParamVal("width", nameVal, 640)); //max 1280
    Camera.setHeight(getParamVal("height", nameVal, 480)); //max 960
    Camera.setBrightness(getParamVal("brightness", nameVal, 50));

    Camera.setSharpness(getParamVal("sharpness", nameVal, 0));
    Camera.setContrast(getParamVal("contrast", nameVal, 0));
    Camera.setSaturation(getParamVal("saturation", nameVal, 0));
    //Camera.setShutterSpeed(getParamVal("shutterspeed", nameVal, 0));
    Camera.setISO(getParamVal("iso", nameVal, 400));
    //Camera.setExposureCompensation(getParamVal("ec", nameVal, 0));

    auto it = nameVal.find("format");
    RASPICAM_FORMAT fmt = (it != end(nameVal)) ?
      getFormatFromString(it->second) :
      RASPICAM_FORMAT_RGB;
    //Camera.setFormat(fmt);
    it = nameVal.find("exposure");
    if (it != end(nameVal))
      Camera.setExposure(getExposureFromString(it->second));

    it = nameVal.find("awb");
    if (it != end(nameVal))
      Camera.setAWB(getAwbFromString(it->second));
    //To Do: unimplemented feature
    //Camera.setAWB_RB(float(getParamVal("awb_b",args ,1)/100.0), float(getParamVal("awb_g",args ,1)/100.0));


  }
  void processCommandLine(Json::Value const& nameVal, camerasp::cam_still &Camera) {
    Camera.setWidth(getParamVal("width", nameVal, 640)); //max 1280
    Camera.setHeight(getParamVal("height", nameVal, 480)); //max 960
    Camera.setBrightness(getParamVal("brightness", nameVal, 50));

    Camera.setSharpness(getParamVal("sharpness", nameVal, 0));
    Camera.setContrast(getParamVal("contrast", nameVal, 0));
    Camera.setSaturation(getParamVal("saturation", nameVal, 0));
    //Camera.setShutterSpeed(getParamVal("shutterspeed", nameVal, 0));
    Camera.setISO(getParamVal("iso", nameVal, 400));
    //Camera.setExposureCompensation(getParamVal("ec", nameVal, 0));

    RASPICAM_FORMAT fmt =  RASPICAM_FORMAT_RGB;
    //Camera.setFormat(fmt);

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
    void save_image(std::vector<unsigned char> const& buffer, std::string const& fName) {
      FILE *fp = nullptr;
      fopen_s(&fp, fName.c_str(), "wb");
      if (fp) {
        for (auto c : buffer) putc(c, fp);
        fclose(fp);
      }
    }
    JSONCPP_STRING readInputTestFile(const char* path)
    {
      std::ifstream ifs(path);
      std::stringstream istr;
      istr << ifs.rdbuf();
      return istr.str();
    }
    Json::Value getDOM(std::string const& path)    {
      JSONCPP_STRING input = readInputTestFile(path.c_str());
      if (input.empty())      {
        throw std::runtime_error("Empty input file");
      }

      Json::Features mode = Json::Features::strictMode();
      mode.allowComments_ = true;
      Json::Value root;

      Json::Reader reader(mode);
      bool parsingSuccessful = reader.parse(input.data(), input.data() + input.size(), root);
      if (!parsingSuccessful)      {
        throw std::runtime_error(
          std::string("Failed to parse file: ") +
          reader.getFormattedErrorMessages());
      }
      return root;
    }
}
