#include <string>
#include <vector>
#ifdef RASPICAM_MOCK
#include "raspicamMock.hpp"
#else
#include <raspicam/raspicam.h>
#endif
void processCommandLine ( std::vector<std::string> const & args,raspicam::RaspiCam &Camera );
