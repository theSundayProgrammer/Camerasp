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
#include <cassert>
#include <csignal>
#include <iostream>
#include <functional>
#include <thread>
#if 0
#include <chrono>

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

int main0()
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
#endif 


int main()
{
  asio::io_service io_service;

  // Prevent io_service from running out of work.
  asio::io_service::work work(io_service);

  // Boost.Asio will register an internal handler for SIGTERM.
  asio::signal_set signal_set(io_service, SIGTERM);
  signal_set.async_wait(
    [&io_service](
      const asio::error_code& error,
      int signal_number)
  {
    std::cout << "Got signal " << signal_number << "; "
      "stopping io_service." << std::endl;
    io_service.stop();
  });

  // Raise SIGTERM.
  std::raise(SIGTERM);

  // By the time raise() returns, Boost.Asio has handled SIGTERM with its
  // own internal handler, queuing it internally.  At this point, Boost.Asio
  // is ready to dispatch this notification to a user signal handler
  // (i.e. those provided to signal_set.async_wait()) within the
  // io_service event loop.
  std::cout << "io_service stopped? " << io_service.stopped() << std::endl;
  assert(false == io_service.stopped());

  // Initiate thread that will run the io_service.  This will invoke
  // the queued handler that is ready for completion.
  std::thread work_thread([&io_service]() { io_service.run(); });

  // Synchornize on the work_thread.  Letting it run to completion.
  work_thread.join();

  // The io_service has been explicitly stopped in the async_wait
  // handler.
  std::cout << "io_service stopped? " << io_service.stopped() << std::endl;
  assert(true == io_service.stopped());
}