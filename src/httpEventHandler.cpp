#include <httpEventHandler.hpp>  
#include <fstream>
#include <jpeg/jpgrdwr.h>
#include <camerasp\parseCmd.hpp>

/// A string to send in responses.
const std::string response_body
(std::string("<html>\r\n") +
  std::string("<head><title>Accepted</title></head>\r\n") +
  std::string("<body><h1>Does not support chunks</h1></body>\r\n") +
  std::string("</html>\r\n"));

std::string filterCommand(std::string const& uri){
  size_t pos = uri.find('/');
  size_t start = (pos == std::string::npos) ? 0 : pos + 1;
  pos = uri.find_first_of("/?", start);
  auto fin = (pos == std::string::npos) ? uri.end() : uri.begin() + pos;
  return std::string(uri.begin() + start, fin);
}

Handlers::Handlers(raspicam::RaspiCam& Camera) : 
  camera_(Camera)
  {}

  void Handlers::request(
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
        std::ifstream t(configPath + "index.html");
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
      std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
  }
  /// The handler for incoming HTTP chunks.
  /// Outputs the chunk header and body to std::cout.
  void Handlers::chunk(
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
  void Handlers::expect_continue(
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
  void Handlers::invalid_request(
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
  void Handlers::connected(http_connection::weak_pointer weak_ptr)
  {
    std::cout << "Connected: " << weak_ptr.lock()->remote_address() << std::endl;
  }

  /// A handler for the signal sent when an HTTP socket is disconnected.
  void Handlers::disconnected(http_connection::weak_pointer weak_ptr)
  {
    std::cout << "Disconnected: " << weak_ptr.lock()->remote_address() << std::endl;
  }

  /// A handler for the signal when a message is sent.
  void Handlers::message_sent(http_connection::weak_pointer) // weak_ptr)
  {
    std::cout << "response sent" << std::endl;
  }



  std::pair<bool, std::vector<unsigned char> > 
    Handlers::getContent(std::string const& uri)
  {
    return std::make_pair(true, Camerasp::imagebuffers[Camerasp::curImg]);
  }
  std::pair<via::http::tx_response, std::string> 
    Handlers::getGETResponse(std::string const& uri)
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
