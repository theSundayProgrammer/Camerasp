//////////////////////////////////////////////////////////////////////////////
// Copyright (c) Joseph Mariadassou
// theSundayProgrammer@gmail.com
// Distributed under the Boost Software License, Version 1.0.
// 
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#ifdef RASPICAM_MOCK
#include "raspicamMock.hpp"
#else
#include <camerasp/cam_still.hpp>
#endif
#include <map>
#ifdef __GNUC__
typedef int errno_t;
errno_t fopen_s(FILE** fp, const char* name, const char* mode);
#endif
#include <asio.hpp>
#include <chrono>
#include <jpeg/jpgrdwr.h>
#include <json/reader.h>
#include <camerasp/commonUtils.hpp>

#ifdef __GNUC__
typedef int errno_t;
errno_t fopen_s(FILE** fp, const char* name, const char* mode);
#endif
namespace Camerasp{
  std::string lowerCase(std::string const& str);
  void processCommandLine(
    Json::Value const &,
    camerasp::cam_still &Camera);
  void processCommandLine(
    std::map<std::string, 
    std::string> const & args, 
    camerasp::cam_still &Camera);
  std::map<std::string, std::string>   tokenize(std::string const& query);
  errno_t readOptions(std::string const& fileName, std::string& options);
  typedef asio::basic_waitable_timer<
    std::chrono::steady_clock>
    high_resolution_timer;
  void setTimer(high_resolution_timer& timer,camerasp::cam_still&);
  std::string getImage(int k);
  void stopCapture();
  void save_image(
    std::vector<unsigned char> const& buffer, 
    std::string const& fName);
  Json::Value getDOM(std::string const& path);
  extern std::chrono::seconds samplingPeriod;
  extern int max_file_count;
  extern std::string  pathname_prefix;

}
