//
// time_t_timer.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>
#include <ctime>
#include <iostream>
#include <chrono>
#include <functional>
#include <camerasp\parseCmd.hpp>
namespace Camerasp {

  std::chrono::seconds samplingPeriod(5);

  std::chrono::time_point<std::chrono::system_clock> prev;
  //std::chrono::time_point<std::chrono::system_clock> finish;
 
  void handle_timeout(const asio::error_code&, high_resolution_timer& timer)  {
    using namespace std::placeholders;
    auto current = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = current - prev;
    std::cout << diff.count() << " s\n";

    if (!quitFlag) { 
      timer.expires_from_now(prev + 2 * samplingPeriod - current);
      prev = current;
      timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer)));
    }
  }


  void setTimer(high_resolution_timer& timer) {
    using namespace std::placeholders;
    try    {
      prev = std::chrono::system_clock::now();
      timer.expires_from_now(samplingPeriod);
      timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer)));
    }  catch (std::exception& e)    {
      std::cout << "Exception: " << e.what() << "\n";
    }
  }

}
