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
#include <raspicam/raspicam.h>
#endif
#include <map>
#include <asio.hpp>
#include <chrono>
#include <jpeg/jpgrdwr.h>
namespace Camerasp
{
  std::string lowerCase(std::string const& str);
  void processCommandLine(std::map<std::string, std::string> const & args, raspicam::RaspiCam &Camera);
  std::map<std::string, std::string>   tokenize(std::string const& query);
  errno_t readOptions(std::string const& fileName, std::string& options);
  typedef asio::basic_waitable_timer<
    std::chrono::high_resolution_clock>
    high_resolution_timer;
  void setTimer(high_resolution_timer& timer,raspicam::RaspiCam&);
  std::vector<unsigned char>  getImage(long k);
  extern int quitFlag;
  //extern unsigned int curImg;
  //extern const unsigned int maxSize;
  //extern std::vector<unsigned char> imagebuffers[];
}
