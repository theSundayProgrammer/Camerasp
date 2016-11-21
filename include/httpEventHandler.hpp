#pragma once
#ifdef RASPICAM_MOCK
#include <camerasp/raspicamMock.hpp>
#else
#include <raspicam/raspicam_still.h>
#endif
#include <via/comms/tcp_adaptor.hpp>
#include <via/http_server.hpp>
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
typedef http_server_type::http_connection_type http_connection;
typedef http_server_type::chunk_type http_chunk_type;
#include <string>
#include <vector>
extern const std::string configPath;
class Handlers {
public:
  Handlers(raspicam::RaspiCam& Camera);

  void request(
    http_connection::weak_pointer weak_ptr,
    via::http::rx_request const& request,
    std::string const& body);
  
  void respond_to_request(http_connection::weak_pointer weak_ptr);
  
  /// The handler for incoming HTTP chunks.
  /// Outputs the chunk header and body to std::cout.
  void chunk(
    http_connection::weak_pointer weak_ptr,
    http_chunk_type const& chunk,
    std::string const& data);
  

  /// A handler for HTTP requests containing an "Expect: 100-continue" header.
  /// Outputs the request and determines whether the request is too big.
  /// It either responds with a 100 CONTINUE or a 413 REQUEST_ENTITY_TOO_LARGE
  /// response.
  void expect_continue(
    http_connection::weak_pointer weak_ptr,
    via::http::rx_request const& request,
    std::string const& /* body */);
  

  /// A handler for the signal sent when an invalid HTTP mesasge is received.
  void invalid_request(
    http_connection::weak_pointer weak_ptr,
    via::http::rx_request const&, // request,
    std::string const& /* body */);
  

  /// A handler for the signal sent when an HTTP socket is connected.
  void connected(http_connection::weak_pointer weak_ptr);
  

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void disconnected(http_connection::weak_pointer weak_ptr);
  

  /// A handler for the signal when a message is sent.
  void message_sent(http_connection::weak_pointer);
  

private:
  std::pair<bool, std::vector<unsigned char> > getContent(std::string const& uri);
  
  std::pair<via::http::tx_response, std::string> getGETResponse(std::string const& uri);
  
  raspicam::RaspiCam& camera_;
};
