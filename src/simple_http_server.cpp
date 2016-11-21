//////////////////////////////////////////////////////////////////////////////
// Copyright (c) Joseph Mariadassou
// theSundayProgrammer@gmail.com
// adapted from Kenneth Baker's via-http
// Distributed under the Boost Software License, Version 1.0.
// 
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include <via/comms/tcp_adaptor.hpp>
#include <via/http_server.hpp>
#include <asio.hpp>
#include <ctime>
#include <iostream>
#include <errno.h>
#ifdef RASPICAM_MOCK
#include <camerasp/raspicamMock.hpp>
#include <stdio.h>
const std::string configPath("./");
#else
#include <raspicam/raspicam_still.h>
typedef int errno_t;
const std::string configPath("/srv/camerasp/options.txt");
#endif
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
int Camerasp::quitFlag = 0;

typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
typedef http_server_type::http_connection_type http_connection;
typedef http_server_type::chunk_type http_chunk_type;


  std::string filterCommand(std::string const& uri)
  {
    size_t pos = uri.find('/');
    size_t start = (pos == std::string::npos) ? 0 : pos + 1;
    pos = uri.find_first_of("/?", start);
    auto fin = (pos == std::string::npos) ? uri.end() : uri.begin() + pos;
    return std::string(uri.begin() + start, fin);
  }
  /// The stop callback function.
  /// Closes the server and all it's connections leaving io_service.run
  /// with no more work to do.
  /// Called whenever a SIGINT, SIGTERM or SIGQUIT signal is received.
  void handle_stop(ASIO_ERROR_CODE const&, // error,
    int, // signal_number,
    http_server_type& http_server)
  {
    std::cout << "Shutting down" << std::endl;
    http_server.shutdown();
  }

  /// A string to send in responses.
  const std::string response_body
  (std::string("<html>\r\n") +
    std::string("<head><title>Accepted</title></head>\r\n") +
    std::string("<body><h1>Does not support chunks</h1></body>\r\n") +
    std::string("</html>\r\n"));


  /**
   * @brief get image from camera andreturn as jpeg
   * @param camera
   * @param uri
   * @return true, data if all went else false,emtpty buffer
   */


   /// The handler for HTTP requests.
   /// Outputs the request.
   /// Responds with 200 OK with the client address in the body.
  class Handlers
  {
  public:
    Handlers(raspicam::RaspiCam& Camera) : camera_(Camera)
    {
    }

    void
      request(
        http_connection::weak_pointer weak_ptr,
        via::http::rx_request const& request,
        std::string const& body)
    {
      std::cout << "\n\nRx request: " << request.uri() << std::endl;

      std::cout << request.headers().to_string() << std::endl;
      std::cout << "Rx body: " << body << std::endl;

      http_connection::shared_pointer connection(weak_ptr.lock());

      if (connection)
      {
        std::string filtered = filterCommand(request.uri());
        std::cout << filtered << std::endl;
        if (filtered == "img.jpg")
        {
          auto resp = getGETResponse(request.uri());
          connection->send(std::move(resp.first), std::move(resp.second));
        }
        else {
          std::ifstream t(configPath+"index.html");
          if (t)
          {
            std::string str(
              (std::istreambuf_iterator<char>(t)),
              std::istreambuf_iterator<char>());
            via::http::tx_response response(via::http::response_status::code::OK);
            response.add_server_header();
            response.add_date_header();
            response.add_header("Content-Type", "text/html");
            response.add_header("charset", "utf-8");
            response.add_content_length_header(str.size());
            connection->send(response, str);
            //std::cout << str << std::endl;
          }
          else {
            via::http::tx_response response(via::http::response_status::code::NOT_FOUND);
            connection->send(response, "Index Not Found");
            std::cout << "index not found" << std::endl;
          }
        }
      }
      else {
        std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
      }
    }
    void respond_to_request(http_connection::weak_pointer weak_ptr)
    {
      http_connection::shared_pointer connection(weak_ptr.lock());
      if (connection)
      {
        // Get the last request on this connection.
        via::http::rx_request const& request(connection->request());

        // Set the default response to 404 Not Found
        via::http::tx_response response(via::http::response_status::code::NOT_FOUND);
        // add the server and date headers
        response.add_server_header();
        response.add_date_header();
        //response.set_status(via::http::response_status::code::OK);
        connection->send(std::move(response),
            via::comms::ConstBuffers(1, asio::buffer(response_body)));
 
      }
      else
        std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
    }
    /// The handler for incoming HTTP chunks.
    /// Outputs the chunk header and body to std::cout.
    void chunk(
      http_connection::weak_pointer weak_ptr,
      http_chunk_type const& chunk,
      std::string const& data)
    {
      if (chunk.is_last())
      {
        std::cout << "Rx chunk is last, extension: " << chunk.extension()
          << " trailers: " << chunk.trailers().to_string() << std::endl;

        // Only send a response to the last chunk.
        respond_to_request(weak_ptr);
      }
      else
        std::cout << "Rx chunk, size: " << chunk.size()
        << " data: " << data << std::endl;
    }

    /// A handler for HTTP requests containing an "Expect: 100-continue" header.
    /// Outputs the request and determines whether the request is too big.
    /// It either responds with a 100 CONTINUE or a 413 REQUEST_ENTITY_TOO_LARGE
    /// response.
    void expect_continue(
      http_connection::weak_pointer weak_ptr,
      via::http::rx_request const& request,
      std::string const& /* body */)
    {
      static const auto MAX_LENGTH(1024);

      std::cout << "expect_continue_handler\n";
      std::cout << "rx request: " << request.to_string();
      std::cout << "rx headers: " << request.headers().to_string() << std::endl;

      // Reject the message if it's too big, otherwise continue
      via::http::tx_response response((request.content_length() > MAX_LENGTH) ?
        via::http::response_status::code::PAYLOAD_TOO_LARGE :
        via::http::response_status::code::CONTINUE);

      http_connection::shared_pointer connection(weak_ptr.lock());
      if (connection)
        connection->send(std::move(response));
      else
        std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
    }

    /// A handler for the signal sent when an invalid HTTP mesasge is received.
    void invalid_request(
      http_connection::weak_pointer weak_ptr,
      via::http::rx_request const&, // request,
      std::string const& /* body */)
    {
      std::cout << "Invalid request from: ";
      http_connection::shared_pointer connection(weak_ptr.lock());
      if (connection)
      {
        std::cout << weak_ptr.lock()->remote_address() << std::endl;
        // Send the default response
        connection->send_response();
        // Disconnect the client
        connection->disconnect();
      }
      else
        std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
    }

    /// A handler for the signal sent when an HTTP socket is connected.
    void connected(http_connection::weak_pointer weak_ptr)
    {
      std::cout << "Connected: " << weak_ptr.lock()->remote_address() << std::endl;
    }

    /// A handler for the signal sent when an HTTP socket is disconnected.
    void disconnected(http_connection::weak_pointer weak_ptr)
    {
      std::cout << "Disconnected: " << weak_ptr.lock()->remote_address() << std::endl;
    }

    /// A handler for the signal when a message is sent.
    void message_sent(http_connection::weak_pointer) // weak_ptr)
    {
      std::cout << "response sent" << std::endl;
    }
  

private:
  std::pair<bool, std::vector<unsigned char> > getContent(std::string const& uri)
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
      std::cout << "Data Size = " << buffer.size() << std::endl;
      return std::make_pair(true, buffer);
    }
    else
      return std::make_pair(false, std::vector<unsigned char>());
  }
  std::pair<via::http::tx_response, std::string> getGETResponse(std::string const& uri)
  {
    auto resp = getContent(uri);
    if (resp.first) {
      // output the request
      via::http::tx_response response(via::http::response_status::code::OK);
      response.add_server_header();
      response.add_date_header();
      response.add_header("Content-Type", "image/jpeg");
      response.add_content_length_header(resp.second.size());
      std::string response_body((char*)(&resp.second[0]), resp.second.size());
      return std::make_pair(response, response_body);
    }
    else {
      via::http::tx_response response(via::http::response_status::code::NOT_FOUND);
      return std::make_pair(response, "Favicon Not Found");
    }
  }
  raspicam::RaspiCam& camera_;
};


typedef int distance;

asio::io_service* pService = nullptr;

#ifndef RASPICAM_MOCK
errno_t fopen_s(FILE** fp, const char* name, const char* mode)
{
  *fp = fopen(name, mode);
  if (*fp) return 0;
  else return -1;
}
#endif


int main(int /* argc */, char* argv[]){
  using namespace std::placeholders;
  std::string app_name(argv[0]);
  unsigned short port_number(8088);
  std::cout << app_name << ": " << port_number << std::endl;
  raspicam::RaspiCam camera;
  try  {
    std::cout << "Connecting to camera" << std::endl;
    std::string options;
    errno_t err = Camerasp::readOptions(configPath + "options.txt", options);
    if (err)     {
      std::cout << "error in opening options file " << std::endl;
    }   else {
      auto opts = Camerasp::tokenize(options);
      Camerasp::processCommandLine(opts, camera);
      if (!camera.open()) {
        std::cerr << "Error opening camera" << std::endl;
        return -1;
      } // The asio io_service.
      asio::io_service io_service;
      pService = &io_service;
      Camerasp::high_resolution_timer timer(io_service);
      Camerasp::setTimer(timer);
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
        std::cerr << "Error: " << error.message() << std::endl;
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
        Camerasp::quitFlag = 1;
        handle_stop(error, signal_number, http_server); 
        io_service.stop();
      });

      // Start the server
      io_service.run();
    }
    

  }  catch (std::exception& e)  {
    std::cerr << "Exception:" << e.what() << std::endl;
    return 1;
  }

  return 0;
}

