//////////////////////////////////////////////////////////////////////////////
// Copyright (c) Joseph Mariadassou
// theSundayProgrammer@gmail.com
// Distributed under the Boost Software License, Version 1.0.
// 
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////

#include <asio.hpp>
#include <ctime>
#include <iostream>
#include <chrono>
#include <functional>
#include <camerasp\parseCmd.hpp>
#include <jpeg/jpgrdwr.h>
#include <mutex>
namespace Camerasp {
  const unsigned int maxSize=100;
  unsigned int curImg=0;
  struct Imagebuffer {
    std::mutex m;
    std::vector<unsigned char> buffer;
  };
  Imagebuffer imagebuffers[maxSize];

  std::chrono::seconds samplingPeriod(1);

  std::chrono::time_point<std::chrono::system_clock> prev;
  //std::chrono::time_point<std::chrono::system_clock> finish;
 
  void handle_timeout(const asio::error_code&, high_resolution_timer& timer, raspicam::RaspiCam& camera_)  {
    using namespace std::placeholders;
    auto current = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = current - prev;
    unsigned int next = (curImg + 1)%maxSize;
    {
      camera_.grab();
      int siz = camera_.getImageBufferSize();
      Camerasp::ImgInfo info;
      info.buffer.resize(siz);
      camera_.retrieve((unsigned char*)(&info.buffer[0]));
      info.image_height = camera_.getHeight();
      std::cout << "H W " << camera_.getHeight() << " " << camera_.getWidth() << std::endl;
      info.image_width = camera_.getWidth();
      info.quality = 100;
      info.row_stride = info.image_width * 3;
      std::vector<unsigned char> buffer;
      if (info.image_height > 0 && info.image_width > 0) {
        info.quality = 100;
        std::cout << "Image Size = " << info.buffer.size() << std::endl;
        buffer = write_JPEG_dat(info);
        imagebuffers[next].m.lock();
        imagebuffers[next].buffer.swap(buffer);
        imagebuffers[next].m.unlock();
        Camerasp::curImg = next;
        std::cout << "Prev Data Size = " << buffer.size() << std::endl;
      }
    }
    if (!quitFlag) { 
      timer.expires_from_now(prev + 2 * samplingPeriod - current);
      prev = current;
      timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer), std::ref(camera_)));
    }
  }


  void setTimer(
    high_resolution_timer& timer,
    raspicam::RaspiCam& camera) {
    using namespace std::placeholders;
    try    {
      prev = std::chrono::system_clock::now();
      timer.expires_from_now(samplingPeriod);
      timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer), std::ref(camera)));
    }  catch (std::exception& e)    {
      std::cout << "Exception: " << e.what() << "\n";
    }
  }
  std::vector<unsigned char>  getImage(long k) {
    unsigned int next = (curImg -k) % maxSize;
    std::vector<unsigned char> imagebuffer;
    imagebuffers[next].m.lock();
    imagebuffer= imagebuffers[next].buffer;
    imagebuffers[next].m.unlock();
    return imagebuffer;
  }
}
