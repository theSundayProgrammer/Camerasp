//////////////////////////////////////////////////////////////////////////////
// Copyright (c) Joseph Mariadassou
// theSundayProgrammer@gmail.com
// Distributed under the Boost Software License, Version 1.0.
// 
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////

#include <asio.hpp>
#include <ctime>
#include <chrono>
#include <functional>
#include <camerasp/parseCmd.hpp>
#include <jpeg/jpgrdwr.h>
#include <mutex>
#include <atomic>


namespace Camerasp {
  const unsigned int maxSize = 100;
  unsigned int curImg = 0;
  std::atomic<int> fileCounter = 0;
  
  struct Imagebuffer {
    std::mutex m;
    std::vector<unsigned char> buffer;
  };
  static Imagebuffer imagebuffers[maxSize];
  static std::atomic<int> currentCount(0);
  static std::chrono::seconds samplingPeriod(3);
  int quitFlag=0;
  static int firstTime = 0;
  static int maxFileCount = 1000;
  std::chrono::time_point<high_resolution_timer::clock_type> prev;
  std::string const pathname_prefix = 
#ifdef __GNUC__
    "/home/pi/data/backup";
#else
   "C:\\Users\\Public\\Pictures\\Camerasp\\backUp";
#endif
  void handle_timeout(
    const asio::error_code&,
    high_resolution_timer& timer,
    raspicam::RaspiCam& camera_) {
    using namespace std::placeholders;
      if (!quitFlag) {
        auto current = high_resolution_timer::clock_type::now();
        std::chrono::duration<double> diff = current - prev;
        unsigned int next = (curImg + 1) % maxSize;
        camera_.grab();
        int siz = camera_.getImageBufferSize();
        Camerasp::ImgInfo info;
        info.buffer.resize(siz);
        camera_.retrieve((unsigned char*)(&info.buffer[0]));
        info.image_height = camera_.getHeight();
        console->debug("Height = {0}, Width= {1}", camera_.getHeight(), camera_.getWidth());
        info.image_width = camera_.getWidth();
        info.quality = 100;
        info.row_stride = info.image_width * 3;
        std::vector<unsigned char> buffer;
        if (info.image_height > 0 && info.image_width > 0) {
          info.quality = 100;
          console->debug("Image Size = {0}", info.buffer.size());
          buffer = write_JPEG_dat(info);
          char intstr[8];
          _itoa(++fileCounter, intstr, 16);
          save_image(buffer, pathname_prefix + intstr + ".jpg");
          {
            std::lock_guard<std::mutex> lock(imagebuffers[next].m);
            imagebuffers[next].buffer.swap(buffer);
          }
          
          fileCounter.compare_exchange_strong(maxFileCount, 0);
          if (currentCount < maxSize) ++currentCount;
          Camerasp::curImg = next;
          console->info("Prev Data Size {0}; time elapse {1}s..", buffer.size(), diff.count());
        }
      timer.expires_at(prev + 2 * samplingPeriod);
      prev = current;
      timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer), std::ref(camera_)));
    }
  }


  void setTimer(
    high_resolution_timer& timer,
    raspicam::RaspiCam& camera_) {
     using namespace std::placeholders;
    try {
      quitFlag = 0;
      prev = high_resolution_timer::clock_type::now();
      timer.expires_at(prev +  samplingPeriod);
      timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer), std::ref(camera_)));
    }
    catch (std::exception& e) {
      console->error("Error: {}..", e.what());
    }
  }
  std::string  getImage(int k) {

    if (k > currentCount && currentCount < maxSize)
      k = currentCount - 1;
    unsigned int next = (curImg - k) % maxSize;
    std::lock_guard<std::mutex> lock(imagebuffers[next].m);
    auto& imagebuffer = imagebuffers[next].buffer;
    return std::string(imagebuffer.begin(), imagebuffer.end());
  }
  void stopCapture()
  {
    quitFlag = 1;
  }
}
