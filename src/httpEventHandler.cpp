//////////////////////////////////////////////////////////////////////////////
// Copyright (c) Joseph Mariadassou
// theSundayProgrammer@gmail.com
// Distributed under the Boost Software License, Version 1.0.
// 
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include <fstream>
#include <jpeg/jpgrdwr.h>
#include <camerasp/parseCmd.hpp>
#include <camerasp/httpEventHandler.hpp>  
#include <regex>
#include <map>
/// A string to send in responses.
const std::string response_body
(std::string("<html>\r\n") +
  std::string("<head><title>Accepted</title></head>\r\n") +
  std::string("<body><h1>Not Implemented</h1></body>\r\n") +
  std::string("</html>\r\n"));


struct UrlParser {
  std::string command;
  std::map<std::string, std::string> queries;
  UrlParser(std::string const& s) {
    std::string uri = s;
    //filter out anything after '#'
    auto pos = uri.find('#');
    if (pos != std::string::npos) {
      uri = uri.erase(pos);
    }
    //detect command which may end with '?' if there are parameters
    std::smatch m;
    std::regex e("([^?]*)(\\??)");
    if (std::regex_search(uri, m, e)) {
      command = m[1];
      //Command detected; now get parameter values "key=value&key=value"
      uri = m.suffix().str();
      std::regex e2("([^&]+)(&?)");
      while (!uri.empty() && std::regex_search(uri, m, e2)) {
        //for each matching key=value save key,value in map
        std::regex e3("([^=]*)(=)");
        std::string keyval = m[1];
        std::smatch m3;
        if (std::regex_search(keyval, m3, e3)) {
          std::string key = m3[1];
          std::string val = m3.suffix().str();
          queries[key] = val;
        }
        uri = m.suffix().str();
      }
    }
    else {
      command = uri;
    }
  }
};
 bool isinteger(const std::string & s, int* k)
{
  if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

  char * p;
  *k = strtol(s.c_str(), &p, 10);

  return (*p == 0);
}
Handlers::Handlers(camerasp::cam_still& Camera) : 
  camera_(Camera)
  {}

  void Handlers::request(
      http_connection::weak_pointer weak_ptr,
      via::http::rx_request const& request,
      std::string const& body)
  {
    //std::cout << "\n\nRx request: " << request.uri() << std::endl;

    //std::cout << "Rx body: " << body << std::endl;

    http_connection::shared_pointer connection(weak_ptr.lock());

    if (connection)
    {
      if (request.uri().empty()) {
        auto resp = getGETResponse(0);
        connection->send(std::move(resp.first), std::move(resp.second));

      }   else {
        UrlParser url(request.uri());
        //std::cout << url.command << std::endl;
        if (url.command == "getImage") {
          int k = 0;
          auto kv = url.queries.begin();
          if (kv->first == "prev") {
            if (kv->second.empty() || !isinteger(kv->second, &k)) {
              k = 0;
            }
          }
          auto resp = getGETResponse(k);
          connection->send(std::move(resp.first), std::move(resp.second));
        }  else if (url.command == "stopCapture"){
          camerasp::stopCapture();
        }  else if (url.command == "startCapture"){
          camerasp::startCapture();
        }  else {
          std::ifstream t(configPath + "index.html");
          if (t) {
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
            ////std::cout << str << std::endl;
          } else {
            via::http::tx_response response(via::http::response_status::code::NOT_FOUND);
            connection->send(response, "Index Not Found");
            //std::cout << "index not found" << std::endl;
          }
        }
      }
    }  else {
      console->error("Failed to lock http_connection::weak_pointer");
    }
  }
  void Handlers::respond_to_request(http_connection::weak_pointer weak_ptr)
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
      console->error("Failed to lock http_connection::weak_pointer");
  }
  /// The handler for incoming HTTP chunks.
  /// Outputs the chunk header and body to //std::cout.
  void Handlers::chunk(
    http_connection::weak_pointer weak_ptr,
    http_chunk_type const& chunk,
    std::string const& data)
  {
    if (chunk.is_last())
    {
      console->debug("Rx chunk is last, extension: {0} trailers {1} ", chunk.extension(), chunk.trailers().to_string());

      // Only send a response to the last chunk.
      respond_to_request(weak_ptr);
    }
  }

  /// A handler for HTTP requests containing an "Expect: 100-continue" header.
  /// Outputs the request and determines whether the request is too big.
  /// It either responds with a 100 CONTINUE or a 413 REQUEST_ENTITY_TOO_LARGE
  /// response.
  void Handlers::expect_continue(
    http_connection::weak_pointer weak_ptr,
    via::http::rx_request const& request,
    std::string const& /* body */)
  {
    static const auto MAX_LENGTH(1024);

    //std::cout << "expect_continue_handler\n";
    //std::cout << "rx request: " << request.to_string();
    //std::cout << "rx headers: " << request.headers().to_string() << std::endl;

    // Reject the message if it's too big, otherwise continue
    via::http::tx_response response((request.content_length() > MAX_LENGTH) ?
      via::http::response_status::code::PAYLOAD_TOO_LARGE :
      via::http::response_status::code::CONTINUE);

    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
      connection->send(std::move(response));
    else
      console->error("Failed to lock http_connection::weak_pointer");
  }

  /// A handler for the signal sent when an invalid HTTP mesasge is received.
  void Handlers::invalid_request(
    http_connection::weak_pointer weak_ptr,
    via::http::rx_request const&, // request,
    std::string const& /* body */)
  {
    //std::cout << "Invalid request from: ";
    http_connection::shared_pointer connection(weak_ptr.lock());
    if (connection)
    {
      //std::cout << weak_ptr.lock()->remote_address() << std::endl;
      // Send the default response
      connection->send_response();
      // Disconnect the client
      connection->disconnect();
    }
    else
      console->error("Failed to lock http_connection::weak_pointer");
  }

  /// A handler for the signal sent when an HTTP socket is connected.
  void Handlers::connected(http_connection::weak_pointer weak_ptr)
  {
    //std::cout << "Connected: " << weak_ptr.lock()->remote_address() << std::endl;
  }

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void Handlers::disconnected(http_connection::weak_pointer weak_ptr)
  {
    //std::cout << "Disconnected: " << weak_ptr.lock()->remote_address() << std::endl;
  }

  /// A handler for the signal when a message is sent.
  void Handlers::message_sent(http_connection::weak_pointer) // weak_ptr)
  {
    //std::cout << "response sent" << std::endl;
  }



  std::pair<via::http::tx_response, std::string> 
    Handlers::getGETResponse(int k)
  {
       // output the request
      via::http::tx_response response(via::http::response_status::code::OK);
      response.add_server_header();
      response.add_date_header();
      console->info("image number={0}",k);
      response.add_header("Content-Type", "image/jpeg");
      std::string responsebody = camerasp::getImage(k);
      response.add_content_length_header(responsebody.size());
      return std::make_pair(response,responsebody );
  }
