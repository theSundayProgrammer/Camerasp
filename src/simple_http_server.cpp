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
#include <iostream>
#include <errno.h>
#ifdef RASPICAM_MOCK
#include <camerasp/raspicamMock.hpp>
#include <stdio.h>
#else
  #include <raspicam/raspicam_still.h>
typedef int errno_t;
#endif
#include <jpeg/jpgrdwr.h>
#include <camerasp/parseCmd.hpp>
#include <algorithm>
#include <cctype>
#include <asio/buffer.hpp>
#include <signal.h>     /* signal, raise, sig_atomic_t */
#include <fstream>
#include <streambuf>
#include <map>
/// Define an HTTP server using std::string to store message bodies
typedef via::http_server<via::comms::tcp_adaptor, std::string> http_server_type;
typedef http_server_type::http_connection_type http_connection;

namespace
{
  std::string lowerCase(std::string const& str)
  {
    std::string retval(str);
    std::transform(std::begin(str), std::end(str), std::begin(retval),tolower);
    return retval;
  }
  std::map<std::string, std::string>
    tokenize(std::string const& query)
  {
    using namespace std;
    map<string, string> result;
    size_t offset = 0;
    size_t pos = query.find('=', offset);
    while (pos != string::npos)
    {
      string name = query.substr(offset, pos - offset);
      offset = pos + 1;
      pos = query.find('&', offset);
      if (pos != string::npos && pos > offset)
      {
        string val = query.substr(offset, pos - offset);
        result.insert(make_pair(lowerCase(name), val));
        offset = pos + 1;
      }
      else {
        string val = query.substr(offset);
        if (!val.empty())
          result.insert(make_pair(lowerCase(name), val));
        break;
      }
      pos = query.find('=', offset);
    }
    return result;
  }
std::string filterCommand(std::string const& uri)
{
  size_t pos = uri.find('/');
  size_t start = (pos == std::string::npos) ? 0 :pos+1;
  pos = uri.find_first_of("/?", start);
  auto fin = (pos == std::string::npos) ? uri.end() : uri.begin() + pos;
  return std::string(uri.begin() + start, fin);
}

/**
 * @brief get image from camera andreturn as jpeg
 * @param camera
 * @param uri
 * @return true, data if all went else false,emtpty buffer
 */


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

      if (connection)
      {
           std::string filtered = filterCommand(request.uri());
          std::cout << filtered << std::endl;
          if (filtered == "img.jpg")
          {
            auto resp = getGETResponse(request.uri());
            connection->send(std::move(resp.first), std::move(resp.second));
          }else {
            std::ifstream t("/srv/camerasp/index.html");
            if (t)
            {
              std::string str((std::istreambuf_iterator<char>(t)),
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
        } else {
          std::cerr << "Failed to lock http_connection::weak_pointer" << std::endl;
        }
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
}

asio::io_service* pService=nullptr;

void my_handler (int param)
{
  if(pService)
	  pService->stop();
}
#ifndef RASPICAM_MOCK
errno_t fopen_s(FILE** fp, const char* name, const char* mode)
{
     *fp=fopen(name,mode);
     if(*fp) return 0;
     else return -1;
}
#endif
errno_t getOptions(char const* fileName, std::string& options)
{
  FILE *fp = nullptr;
  errno_t err = fopen_s(&fp, fileName, "r");
  if (err==0)
  {
    for (int c = getc(fp); c != EOF ; c = getc(fp))
    {
      if (c != '\n') options.push_back(c);
    }
    fclose(fp);
  }
  return err;
}
int main(int /* argc */, char* argv[])
{
    std::string app_name(argv[0]);
    unsigned short port_number(8088);
    std::cout << app_name << ": " << port_number << std::endl;
    raspicam::RaspiCam camera;
	void (*prev_handler)(int) = signal (SIGTERM, my_handler);
    try
    {
        std::cout << "Connecting to camera" << std::endl;
        std::string options;
        errno_t err = getOptions("/srv/camerasp/options.txt", options);
        if (err == 0) {
           auto opts = tokenize(options);
          processCommandLine(opts, camera);
          if (!camera.open()) {
            std::cerr << "Error opening camera" << std::endl;
            return -1;
          } // The asio io_service.
          asio::io_service io_service;
          pService = &io_service;

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
        else {
          std::cout << "error in opening options file " << std::endl;
        }

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception:" << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#ifdef RASPICAM_MOCK
#include <camerasp/raspicamMock.hpp>
#include <stdio.h>

namespace Camerasp
{
  std::vector<unsigned char> write_JPEG_dat(struct Camerasp::ImgInfo const &dat)
  {
    std::vector<unsigned char> buffer; 
    FILE *fp = nullptr;
    fopen_s(&fp, "C:\\Users\\Public\\Pictures\\figure_3.jpg", "rb");
    if (fp)
    {
      for (int c = getc(fp); c != EOF; c = getc(fp))
      {
        buffer.push_back(c);
      }
      fclose(fp);
    }
    return buffer;

  }
}
#endif
