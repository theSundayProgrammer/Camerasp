#include <string>
#include <vector>
#ifdef RASPICAM_MOCK
#include "raspicamMock.hpp"
#else
#include <raspicam/raspicam.h>
#endif
#include <map>
void processCommandLine (std::map<std::string, std::string> const & args,raspicam::RaspiCam &Camera );
