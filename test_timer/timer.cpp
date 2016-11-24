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
typedef asio::basic_waitable_timer<
    std::chrono::steady_clock>
  high_resolution_timer;

std::chrono::seconds period(1);
std::chrono::time_point<std::chrono::steady_clock> prev, finish;
std::string lowerCase(std::string const& str)
{
  std::string retval(str);
  std::transform(std::begin(str), std::end(str), std::begin(retval), tolower);
  return retval;
}

//void handle_timeout(const asio::error_code&, high_resolution_timer& timer)
//{
//  using namespace std::placeholders;
//  auto current = std::chrono::system_clock::now();
//  std::chrono::duration<double> diff = current - prev;
//  std::cout << diff.count() << " s\n";
//
//  if (current < finish)
//  {
//    timer.expires_from_now(prev + 2 * period - current);
//    prev = current;
//    timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer)));
//  }
//}
void handle_timeout(const asio::error_code&, high_resolution_timer& timer)
{
  using namespace std::placeholders;
  auto current = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = current - prev;
  std::cout << diff.count() << " s\n";

  if (current < finish)
  {
    auto next = prev + 2 * period;
    timer.expires_at(next);
    prev = current;
    timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer)));
  }
}

int main()
{
  using namespace std::placeholders;
  try
  {
    asio::io_context io_context;
    prev = std::chrono::steady_clock::now();
    high_resolution_timer timer(io_context);
    finish = prev + std::chrono::seconds(10);
    timer.expires_from_now(period);
    //std::cout << "Starting synchronous wait\n";
    //timer.wait();
    //std::cout << "Finished synchronous wait\n";

    //timer.expires_from_now(std::chrono::seconds(5));
    std::cout << "Starting asynchronous wait\n";
    timer.async_wait(std::bind(&handle_timeout, _1, std::ref(timer)));
    io_context.run();
    std::cout << "Finished asynchronous wait\n";
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}
