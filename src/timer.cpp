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
#include <stdlib.h>
namespace {
  const unsigned int maxSize = 100;
  std::atomic<int> pending_count;
  unsigned int curImg = 0;
  std::atomic<int> fileCounter;
  typedef std::vector<unsigned char> buffer_t;
  struct Imagebuffer {
    std::mutex m;
    std::vector<unsigned char> buffer;
  };
  static Imagebuffer imagebuffers[maxSize];
  //cuurrenCount is used on startup to avoid a request for an unfilled buffer
  static std::atomic<int> currentCount(0);
  int quitFlag = 0;
  std::chrono::time_point<camerasp::high_resolution_timer::clock_type> prev;
  void save_image(std::vector<unsigned char> const& buffer, std::string const& fName) {
    FILE *fp = nullptr;
    fopen_s(&fp, fName.c_str(), "wb");
    if (fp) {
      for (auto c : buffer) putc(c, fp);
      fclose(fp);
    }
  }


  //need a better way of handling file save
   void save_file(int file_number) {
    buffer_t buffer;
    {
      std::lock_guard<std::mutex> lock(imagebuffers[file_number].m);
      buffer = imagebuffers[file_number].buffer;
    }
    char intstr[8];
    sprintf(intstr, "%04d", file_number);
    save_image(buffer, camerasp::pathname_prefix + intstr + ".jpg");
    --pending_count;
  }

  std::vector<unsigned char> grabPicture(camerasp::cam_still& camera_) {
    //At any point in time only one instance of this function will be running
    camerasp::ImgInfo info;
    console->debug("Height = {0}, Width= {1}", camera_.getHeight(), camera_.getWidth());
    auto siz = camera_.getImageBufferSize();
    info.buffer.resize(siz);
    camera_.takePicture((unsigned char*)(&info.buffer[0]), &siz);
    info.image_height = camera_.getHeight();
    info.image_width = camera_.getWidth();
    info.quality = 100;
    info.row_stride = info.image_width * 3;
    std::vector<unsigned char> buffer;

    if (info.image_height > 0 && info.image_width > 0) {
      info.quality = 100;
      console->debug("Image Size = {0}", info.buffer.size());
      buffer = write_JPEG_dat(info);
    }
    return buffer;
  }
}
namespace camerasp {
  std::chrono::seconds samplingPeriod;
  int max_file_count;
  std::string  pathname_prefix;

  void handle_timeout(
    const asio::error_code&,
    high_resolution_timer& timer,
    camerasp::cam_still& camera_) {
    //At any point in time only one instance of this function will be running
    using namespace std::placeholders;

    if (!quitFlag) {
      auto current = high_resolution_timer::clock_type::now();
      std::chrono::duration<double> diff = current - prev;
      unsigned int next = curImg;
      auto buffer = grabPicture(camera_);
      {
        std::lock_guard<std::mutex> lock(imagebuffers[next].m);
        imagebuffers[next].buffer.swap(buffer);
      }
      int files_in_queue = ++pending_count;
      if (files_in_queue < max_file_count / 10 || files_in_queue < 10)
      {
        asio::post(timer.get_io_context(), std::bind(save_file, next));
      }
      else
      {
        --pending_count;
      }
      if (currentCount < maxSize) ++currentCount;
      curImg = (curImg + 1) % maxSize;
      //console->info("Prev Data Size {0}; time elapse {1}s..", buffer.size(), diff.count());
      timer.expires_at(prev + 2 * samplingPeriod);
      prev = current;
      timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer), std::ref(camera_)));
    }
  }


  void setTimer(
    high_resolution_timer& timer,
    camerasp::cam_still& camera_) {
    using namespace std::placeholders;
    try {
      prev = high_resolution_timer::clock_type::now();
      timer.expires_at(prev + samplingPeriod);
      timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer), std::ref(camera_)));
    }
    catch (std::exception& e) {
      console->error("Error: {}..", e.what());
    }
  }
  std::string  getImage(int k) {

    if (k > currentCount && currentCount < maxSize)
      k = currentCount - 1;
    int next = (curImg - 1 - k) % maxSize;
    console->info("Image number = {0}", next);
    std::lock_guard<std::mutex> lock(imagebuffers[next].m);
    auto& imagebuffer = imagebuffers[next].buffer;
    return std::string(imagebuffer.begin(), imagebuffer.end());
  }
  void startCapture()
  {
    quitFlag = 0;
  }
  void stopCapture()
  {
    quitFlag = 1;
  }
}
