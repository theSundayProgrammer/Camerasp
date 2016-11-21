#include <string>
#include <vector>
#ifdef RASPICAM_MOCK
#include "raspicamMock.hpp"
#else
#include <raspicam/raspicam.h>
#endif
#include <map>
namespace Camerasp
{
  std::string lowerCase(std::string const& str);
  void processCommandLine(std::map<std::string, std::string> const & args, raspicam::RaspiCam &Camera);
  std::map<std::string, std::string>   tokenize(std::string const& query);

}
