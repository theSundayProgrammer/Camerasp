//////////////////////////////////////////////////////////////////////////////
// Copyright (c) Joseph Mariadassou
// theSundayProgrammer@gmail.com
// adapted from Kenneth Baker's via-http
// Distributed under the Boost Software License, Version 1.0.
// 
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include <asio.hpp>
#include <ctime>
//#include <iostream>
#include <errno.h>

#include <jpeg/jpgrdwr.h>
#include <camerasp/parseCmd.hpp>
#include <algorithm>
#include <cctype>
#include <asio/buffer.hpp>
#include <signal.h>     /* signal, raise, sig_atomic_t */
#include <fstream>
#include <functional>
#include <streambuf>
#include <map>
#include <camerasp/httpEventHandler.hpp>
std::string logpath;
#ifdef RASPICAM_MOCK
#include <camerasp/raspicamMock.hpp>
#include <stdio.h>
const std::string configPath="./";
#else
#include <raspicam/raspicam_still.h>
typedef int errno_t;
const std::string configPath="/srv/camerasp/";
#endif
std::shared_ptr<spdlog::logger> console;


  /// The stop callback function.
  /// Closes the server and all it's connections leaving io_service.run
  /// with no more work to do.
  /// Called whenever a SIGINT, SIGTERM or SIGQUIT signal is received.
  void handle_stop(ASIO_ERROR_CODE const&, // error,
    int, // signal_number,
    http_server_type& http_server)
  {
    console->info("shutting down ");
    http_server.shutdown();
  }


  /**
   * @brief get image from camera andreturn as jpeg
   * @param camera
   * @param uri
   * @return true, data if all went else false,emtpty buffer
   */


  


asio::io_service* pService = nullptr;

#ifdef __GNUC__
//fopen_s is not defined in gcc.
errno_t fopen_s(FILE** fp, const char* name, const char* mode)
{
  *fp = fopen(name, mode);
  if (*fp) return 0;
  else return -1;
}
#endif


int main(int /* argc */, char* argv[]){
  using namespace std::placeholders;
  namespace spd = spdlog;
  std::string app_name(argv[0]);
  Json::Value root = Camerasp::getDOM(configPath + "options.json");
  Json::Value log_config = root["Logging"];
  Json::Value json_path = log_config["path"];
  logpath = json_path.asString();
  int size_mega_bytes = log_config["size"].asInt();
  int count_files = log_config["count"].asInt();
  console = spd::rotating_logger_mt("console", logpath, 1024*1024* size_mega_bytes, count_files);

  Json::Value backup = root["Data"];
  Camerasp::max_file_count = backup["count"].asInt();
  int secs = backup["sample_period"].asInt();
  Camerasp::samplingPeriod = std::chrono::seconds(secs);
  Camerasp::pathname_prefix = backup["path_prefix"].asString();
  //console = spd::stdout_color_mt("console");
  console->set_level(spdlog::level::info);
  unsigned short port_number(8088);
  raspicam::RaspiCam camera;
  try  {
    console->info("Welcome to spdlog!");
      Json::Value camera_config = root["Camera"];
      if (camera_config.empty())
      {
        console->critical("Unable to read Json");
        return 1;
      }
      Camerasp::processCommandLine(camera_config, camera);
      if (!camera.open()) {
        console->critical("Error opening camera");
        return -1;
      } // The asio io_service.
      asio::io_service io_service;
      pService = &io_service;
      //Begin Camera Capture
      Camerasp::high_resolution_timer timer(io_service);
      Camerasp::setTimer(timer,camera);
      // Create the HTTP server, attach the request handler
      http_server_type http_server(io_service);
      Handlers handler(camera);
      { // todo: refactor to an other function
        http_server.request_received_event(std::bind(&Handlers::request, handler, _1, _2, _3));
        http_server.chunk_received_event(std::bind(&Handlers::chunk, handler, _1, _2, _3));
        http_server.request_expect_continue_event(std::bind(&Handlers::expect_continue, handler, _1, _2, _3));
        http_server.invalid_request_event(std::bind(&Handlers::invalid_request, handler, _1, _2, _3));
        http_server.socket_connected_event(std::bind(&Handlers::connected, handler, _1));

        http_server.socket_disconnected_event(std::bind(&Handlers::disconnected, handler, _1));
        http_server.message_sent_event(std::bind(&Handlers::message_sent, handler, _1));

        // set the connection timeout (10 seconds)
        http_server.set_timeout(10000);

        // set the connection buffer sizes
        http_server.set_rx_buffer_size(16384);
        http_server.tcp_server()->set_receive_buffer_size(16384);
        http_server.tcp_server()->set_send_buffer_size(16384);
      }
      // start accepting http connections on the port
      ASIO_ERROR_CODE error(http_server.accept_connections(port_number));
      if (error)    {
        console->critical ("Error: {0}", error.message());
        return 1;
      }

      // The signal set is used to register termination notifications
      asio::signal_set signals_(io_service);
#ifndef RASPICAM_MOCK
      signals_.add(SIGINT);
#endif
      signals_.add(SIGTERM);
#if defined(SIGQUIT)
      signals_.add(SIGQUIT);
#endif // #if defined(SIGQUIT)

      // register the handle_stop callback
      signals_.async_wait([&]
      (ASIO_ERROR_CODE const& error, int signal_number)  { 
        Camerasp::stopCapture();
        handle_stop(error, signal_number, http_server); 
        io_service.stop();
      });
      // Start the on two  worker threads server
      std::thread thread1{ [&io_service]() { io_service.run(); } };
      std::thread thread2{ [&io_service]() { io_service.run(); } };
      io_service.run();
      thread1.join();
      thread2.join();
    
    

  }  catch (std::exception& e)  {
    console->error("Exception: {0}", e.what());
    return 1;
  }
  spd::drop_all();
  return 0;
}

