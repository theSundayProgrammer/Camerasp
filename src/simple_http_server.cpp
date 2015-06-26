//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include <via/comms/tcp_adaptor.hpp>
#include <via/http_server.hpp>
#include <iostream>
#include <raspicam/raspicam.h>
#include <raspicam/parseCmd.h>
#include <algorithm>
#include <jpeg/jpgrdwr.h>
#include <cctype>
#include <asio/buffer.hpp>

/// Define an HTTP server using std::string to store message bodies
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
typedef http_server_type::http_connection_type http_connection;

namespace
{
  template < class ContainerT >
void tokenize( ContainerT& tokens,
               const std::string& str,
               const std::string& delimiters)
{
    std::string::size_type lastPos = 0;

    using value_type = typename ContainerT::value_type;
    using size_type = typename ContainerT::size_type;
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);
    while (pos != std::string::npos) {

        if (pos != lastPos )
            tokens.push_back(value_type(str.data() + lastPos, (size_type)pos - lastPos));

        lastPos = pos + 1;
        pos = str.find_first_of(delimiters, lastPos);
    }

    pos = str.length();

    if (pos != lastPos )
        tokens.push_back(value_type(str.data() + lastPos, (size_type)pos - lastPos));
}  
    
std::vector<std::string> splitUrl(std::string const& url)
{
    std::vector<std::string> strs;
    tokenize(strs, url, "\t .+?:= ");
    return strs;
}
/**
 * @brief get image from camera andreturn as jpeg
 * @param camera
 * @param uri
 * @return true, data if all went else false,emtpty buffer
 */
std::pair<bool, std::vector<unsigned char> > getContent(raspicam::RaspiCam& camera, std::string const& uri)
{


    

    camera.grab();
    int siz = camera.getImageBufferSize();
    Camerasp::ImgInfo info;
    info.buffer.resize(siz);
    camera.retrieve((unsigned char*)(&info.buffer[0]));
    info.image_height = camera.getHeight();
    std::cout << "H W " << camera.getHeight() << " " << camera.getWidth() << std::endl;
    info.image_width = camera.getWidth();
    info.quality = 100;
    info.row_stride = info.image_width * 3;
    std::vector<unsigned char> buffer;
    if (info.image_height > 0 && info.image_width > 0) {
        info.quality = 100;
        std::cout << "Image Size = " << info.buffer.size() << std::endl;
        buffer = write_JPEG_dat(info);
        std::cout << "Data Size = " << buffer.size() << std::endl;
        return std::make_pair(true, buffer);
    } else
        return std::make_pair(false, std::vector<unsigned char>());
}

/// The handler for HTTP requests.
/// Outputs the request.
/// Responds with 200 OK with the client address in the body.
class request_handler
{
public:
    request_handler(raspicam::RaspiCam& Camera) : camera_(Camera)
    {
    }

    void
    operator()(http_connection::weak_pointer weak_ptr, via::http::rx_request const& request, std::string const& body)
    {
        std::cout << "\n\nRx request: " << request.uri() << std::endl;

        std::cout << request.headers().to_string() << std::endl;
        std::cout << "Rx body: " << body << std::endl;

        http_connection::shared_pointer connection(weak_ptr.lock());
        if (connection) {
            // output the request
            via::http::tx_response response(via::http::response_status::code::OK);
            response.add_server_header();
            response.add_date_header();
            auto resp = getContent(camera_, request.uri());
            if (resp.first) {
                response.add_header("Content-Type", "image/jpeg");
                response.add_content_length_header(resp.second.size());
                std::string response_body((char*)(&resp.second[0]), resp.second.size());
                connection->send(std::move(response), std::move(response_body));
            } else {
                // respond with the client's address
                via::http::tx_response resp(via::http::response_status::code::NOT_FOUND);
                connection->send(resp, "Favicon not implemented");
            }

        } else
            std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
    }

private:
    raspicam::RaspiCam& camera_;
};
}

int main(int /* argc */, char* argv[])
{
    std::string app_name(argv[0]);
    unsigned short port_number(8088);
    std::cout << app_name << ": " << port_number << std::endl;
    raspicam::RaspiCam camera;
    try
    {
        std::cout << "Connecting to camera" << std::endl;
        const char* options="H=480+W=640";
        std::vector<std::string> opts=splitUrl(options);
        processCommandLine(opts, camera);
       if (!camera.open()) {
            std::cerr << "Error opening camera" << std::endl;
            return -1;
        } // The asio io_service.
        asio::io_service io_service;

        // Create the HTTP server, attach the request handler
        http_server_type http_server(io_service);

        http_server.request_received_event(request_handler(camera));

        // Accept IPV4 connections on port_number
        std::error_code error(http_server.accept_connections(port_number, true));
        if (error) {
            std::cerr << "Error: " << error.message() << std::endl;
            return 1;
        }

        // Start the server
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception:" << e.what() << std::endl;
        return 1;
    }

    return 0;
}
