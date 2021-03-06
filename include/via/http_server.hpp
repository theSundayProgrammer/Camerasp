#ifndef HTTP_SERVER_HPP_VIA_HTTPLIB_
#define HTTP_SERVER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2015 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file http_server.hpp
/// @brief Contains the http_server template class.
//////////////////////////////////////////////////////////////////////////////
/// @mainpage via-httplib: A C++ HTTP Library
///
/// @section server_sec Servers
///
/// Servers receive HTTP requests into via::http::request_receiver and send
/// HTTP responses in a via::http::tx_response.
///
/// @subsection http_server_sec HTTP Server
///
/// An HTTP server class is declared by instantiating the via::http_server
/// template class with via::comms::tcp_adaptor.
///
/// @subsection https_server_sec HTTPS Server
///
/// An HTTPS server class is declared by instantiating the via::http_server
/// template class with via::comms::ssl::ssl_tcp_adaptor.
///
/// @section client_sec Clients
///
/// Clients send HTTP requests in a via::http::tx_request and receive
/// HTTP responses in a via::http::response_receiver.
///
/// @subsection http_client_sec HTTP Client
///
/// A HTTP client class is declared by instantiating the via::http_client
/// template class with via::comms::tcp_adaptor.
///
/// @subsection https_client_sec HTTPS Client
///
/// A HTTP client class is declared by instantiating the via::http_client
/// template class with via::comms::ssl::ssl_tcp_adaptor.
///
//////////////////////////////////////////////////////////////////////////////
#include "http_connection.hpp"
#include "via/comms/server.hpp"
#include "via/http/request_router.hpp"
#ifdef HTTP_SSL
#include <boost/asio/ssl/context.hpp>
#endif
#include <map>
#include <stdexcept>
//#include <iostream>

namespace via
{
  ////////////////////////////////////////////////////////////////////////////
  /// @class http_server
  /// The class template can be configured to use either tcp or ssl sockets
  /// depending upon which class is provided as the SocketAdaptor:
  /// tcp_adaptor or ssl::ssl_tcp_adaptor respectively.
  /// @see comms::tcp_adaptor
  /// @see comms::ssl::ssl_tcp_adaptor
  /// @param SocketAdaptor the type of socket to use:
  /// tcp_adaptor or ssl::ssl_tcp_adaptor
  /// @param Container the container to use for the rx & tx buffers:
  /// std::vector<char> (the default) or std::string.
  /// @param use_strand for multi-threaded
  /// if true use an asio::strand to wrap the handlers,
  /// default false.
  ////////////////////////////////////////////////////////////////////////////
  template <typename SocketAdaptor, typename Container = std::vector<char>,
            bool use_strand = false>
  class http_server
  {
  public:

    /// The comms server for the underlying connections, TCP or SSL.
    typedef comms::server<SocketAdaptor, Container, use_strand> server_type;

    /// The http_connections managed by this server.
    typedef http_connection<SocketAdaptor, Container, use_strand>
      http_connection_type;

    /// The underlying comms connection, TCP or SSL.
    typedef typename http_connection_type::connection_type connection_type;

    /// A collection of http_connections keyed by the connection pointer.
    typedef std::map<void*, std::shared_ptr<http_connection_type> >
      connection_collection;

    /// The template requires a typename to access the iterator.
    typedef typename connection_collection::iterator
      connection_collection_iterator;

    /// The template requires a typename to collection value_type.
    typedef typename connection_collection::value_type
      connection_collection_value_type;

    /// The template requires a typename to access the iterator.
    typedef typename Container::const_iterator Container_const_iterator;

    /// The type of the request_receiver.
    typedef typename http::request_receiver<Container> http_request;

    /// The chunk type
    typedef typename http::rx_chunk<Container> chunk_type;

    /// The RequestHandler type.
    typedef std::function <void (std::weak_ptr<http_connection_type>,
                                 http::rx_request const&, Container const&)>
      RequestHandler;

    /// The ChunkHandler type.
    typedef std::function <void (std::weak_ptr<http_connection_type>,
                                 chunk_type const&, Container const&)>
      ChunkHandler;

    /// The ConnectionHandler type.
    typedef std::function <void (std::weak_ptr<http_connection_type>)>
      ConnectionHandler;

    /// The built-in request_router type.
    typedef typename http::request_router<Container> request_router_type;

    /// The built-in request_router Handler type.
    typedef typename request_router_type::Handler request_router_handler_type;

  private:

    ////////////////////////////////////////////////////////////////////////
    // Variables

    std::shared_ptr<server_type> server_;    ///< the communications server
    connection_collection http_connections_; ///< the communications channels
    request_router_type   request_router_;   ///< the built-in request_router
    bool                  shutting_down_;    ///< the server is shutting down

    // Request parser parameters
    bool           strict_crlf_;       ///< enforce strict parsing of CRLF
    unsigned char  max_whitespace_;    ///< the max no of consectutive whitespace characters.
    unsigned char  max_method_length_; ///< the maximum length of a request method
    size_t         max_uri_length_;    ///< the maximum length of a uri.
    unsigned short max_line_length_;   ///< the max length of a field line
    unsigned short max_header_number_; ///< the max no of header fields
    size_t         max_header_length_; ///< the max cumulative length
    size_t         max_body_size_;     ///< the maximum size of a request body
    size_t         max_chunk_size_;    ///< the maximum size of a request chunk

    // HTTP server options
    bool require_host_header_; ///< whether the http server requires a host header
    bool translate_head_;      ///< whether the http server translates HEAD requests
    bool trace_enabled_;       ///< whether the http server responds to TRACE requests
    bool auto_disconnect_;     ///< whether the http server disconnects invalid requests

    // callback function pointers
    RequestHandler    http_request_handler_; ///< the request callback function
    ChunkHandler      http_chunk_handler_;   ///< the http chunk callback function
    RequestHandler    http_continue_handler_;///< the continue callback function
    RequestHandler    http_invalid_handler_; ///< the invalid callback function
    ConnectionHandler connected_handler_;    ///< the connected callback function
    ConnectionHandler disconnected_handler_; ///< the disconncted callback function
    ConnectionHandler message_sent_handler_; ///< the packet sent callback function

    ////////////////////////////////////////////////////////////////////////
    // Functions

    /// Handle a connected signal from an underlying comms connection.
    /// @param connection a weak ponter to the underlying comms connection.
    void connected_handler(std::weak_ptr<connection_type> connection)
    {
      // Use the raw pointer of the connection as the map key.
      void* pointer(connection.lock().get());
      if (!pointer)
        return;

      // search for the connection in the collection
      connection_collection_iterator iter(http_connections_.find(pointer));
      if (iter == http_connections_.end())
      {
        // Create and configure a new http_connection_type.
        std::shared_ptr<http_connection_type> http_connection
        (new http_connection_type(connection,
          strict_crlf_,
          max_whitespace_,
          max_method_length_,
          max_uri_length_,
          max_line_length_,
          max_header_number_,
          max_header_length_,
          max_body_size_,
          max_chunk_size_));

        http_connection->set_translate_head(translate_head_);
        http_connection->set_concatenate_chunks(!http_chunk_handler_);

        http_connections_.insert
        (connection_collection_value_type(pointer, http_connection));
        // signal that the socket is connected
        if (connected_handler_)
          connected_handler_(http_connection);
      }
      else
        spdlog::get("console")->error("http_server, error: duplicate connection for {0}", iter->second->remote_address());
    }

    /// Route the request using the request_router_.
    /// @param weak_ptr a weak pointer to the comms connection.
    /// @param request the received request.
    /// @param body the received request body.
    void route_request(std::weak_ptr<http_connection_type> weak_ptr,
                       http::rx_request const& request,
                       Container const& body)
    {
      std::shared_ptr<http_connection_type> connection(weak_ptr.lock());
      if (connection)
      {
        Container response_body;
        http::tx_response response
            (request_router_.handle_request(request, body, response_body));
        response.add_date_header();
        response.add_server_header();
        connection->send(std::move(response), std::move(response_body));
      }
    }

    /// Receive data packets on an underlying communications connection.
    /// @param iter a valid iterator into the connection collection.
    void receive_handler(connection_collection_iterator conn)
    {
      // Get the connection
      std::shared_ptr<http_connection_type> http_connection(conn->second);

      // Get the receive buffer
      Container const& rx_buffer(http_connection->read_rx_buffer());
      Container_const_iterator iter(rx_buffer.begin());
      Container_const_iterator end(rx_buffer.end());

      // Get the receive parser for this connection
      http::Rx rx_state(http::RX_VALID);

      // Loop around the received buffer while there's valid data to read
      while ((iter != end) && (rx_state != http::RX_INVALID))
      {
        rx_state = http_connection->rx().receive(iter, end);

        switch (rx_state)
        {
        case http::RX_VALID:
          // If it's NOT a TRACE request
          if (!http_connection->request().is_trace())
          {
            http_request_handler_(http_connection,
                                  http_connection->request(),
                                  http_connection->body());
            if (!http_connection->request().is_chunked())
              http_connection->rx().clear();
            break;
          }
          else if (trace_enabled_) // the server reflects the message back.
          {
            // Response is OK with a Content-Type: message/http header
            // The body of the response contains the TRACE request
            http::tx_response ok_response(http::response_status::code::OK);
            ok_response.add_content_http_header();
            http_connection->send(ok_response,
                                  http_connection->rx().trace_body());
            http_connection->rx().clear();
            break;
          }
          // intentional fall through

        case http::RX_INVALID:
          if (http_invalid_handler_)
            http_invalid_handler_(http_connection,
                                  http_connection->request(),
                                  http_connection->body());
          else
          {
            http_connection->send_response();
            if (auto_disconnect_)
              http_connection->disconnect();
          }
          http_connection->rx().clear();
          break;

        case http::RX_EXPECT_CONTINUE:
          if (http_continue_handler_)
            http_continue_handler_(http_connection,
                                   http_connection->request(),
                                   http_connection->body());
          else
            http_connection->send_response();
          break;

        case http::RX_CHUNK:
          if (http_chunk_handler_)
            http_chunk_handler_(http_connection,
                                http_connection->chunk(),
                                http_connection->chunk().data());
          if (http_connection->chunk().is_last())
            http_connection->rx().clear();
          break;

        default:
          break;
        } // end switch
      } // end while
    }

    /// Handle a disconnected signal from an underlying comms connection.
    /// Noitfy the handler and erase the connection from the collection.
    /// @param iter a valid iterator into the connection collection.
    void disconnected_handler(connection_collection_iterator iter)
    {
      // Noitfy the disconnected handler if one exists
      if (disconnected_handler_)
        disconnected_handler_(iter->second);

      http_connections_.erase(iter);

      // If the http_server is being shutdown and this was the last connection
      if (shutting_down_ && http_connections_.empty())
        server_->close();
    }

    /// Receive an event from the underlying comms connection.
    /// @param event the type of event.
    /// @param connection a weak ponter to the underlying comms connection.
    void event_handler(int event, std::weak_ptr<connection_type> connection)
    {
      if (via::comms::CONNECTED == event)
        connected_handler(connection);
      else
      {
        // Get the raw pointer of the connection
        void* pointer(connection.lock().get());
        if (!pointer)
          return;

        // search for the connection in the collection
        connection_collection_iterator iter(http_connections_.find(pointer));
        if (iter == http_connections_.end())
        {
          spdlog::get("console")->error("http_server, event_handler error: connection not found ");
          return;
        }

        switch(event)
        {
        case via::comms::RECEIVED:
          receive_handler(iter);
          break;
        case via::comms::SENT:
          // Noitfy the sent handler if one exists
          if (message_sent_handler_)
            message_sent_handler_(iter->second);
          break;
        case via::comms::DISCONNECTED:
          disconnected_handler(iter);
          break;
        default:
          ;
        }
      }
    }

    /// Receive an error from the underlying comms connection.
    /// @param error the boost error_code.
    // @param connection a weak ponter to the underlying comms connection.
    void error_handler(const ASIO_ERROR_CODE &ec,
                       std::weak_ptr<connection_type>) // connection)
    {
      console->error("error_handler {0} : {1}", ec.category().name(), ec.value());
    }

    ////////////////////////////////////////////////////////////////////////

  public:

    /// Copy constructor deleted to disable copying.
    http_server(http_server const&) = delete;

    /// Assignment operator deleted to disable copying.
    http_server& operator=(http_server) = delete;

    /// Constructor.
    /// @param io_service a reference to the ASIO::io_service.
    /// @param auth_ptr a shared pointer to an authentication.
    explicit http_server(ASIO::io_service& io_service) :
      server_(new server_type(io_service)),
      http_connections_(),
      request_router_(),
      shutting_down_(false),

      // Set request parser parameters to default values
      strict_crlf_        (false),
      max_whitespace_     (http_request::DEFAULT_MAX_WHITESPACE_CHARS),
      max_method_length_  (http_request::DEFAULT_MAX_METHOD_LENGTH),
      max_uri_length_     (http_request::DEFAULT_MAX_URI_LENGTH),
      max_line_length_    (http_request::DEFAULT_MAX_LINE_LENGTH),
      max_header_number_  (http_request::DEFAULT_MAX_HEADER_NUMBER),
      max_header_length_  (http_request::DEFAULT_MAX_HEADER_LENGTH),
      max_body_size_      (http_request::DEFAULT_MAX_BODY_SIZE),
      max_chunk_size_     (http_request::DEFAULT_MAX_CHUNK_SIZE),

      require_host_header_(true),
      translate_head_     (true),
      trace_enabled_      (false),
      auto_disconnect_    (false),

      http_request_handler_ (),
      http_chunk_handler_   (),
      http_continue_handler_(),
      http_invalid_handler_ (),
      connected_handler_    (),
      disconnected_handler_ (),
      message_sent_handler_ ()
    {
      server_->set_event_callback([this]
        (int event, std::weak_ptr<connection_type> connection)
          { event_handler(event, connection); });
      server_->set_error_callback([this]
        (ASIO_ERROR_CODE const& error,
         std::weak_ptr<connection_type> connection)
          { error_handler(error, connection); });
      // Set no delay, i.e. disable the Nagle algorithm
      // An http_server will want to send messages immediately
      server_->set_no_delay(true);
    }

    /// Destructor, close the connections.
    ~http_server()
    { close(); }

    /// Start accepting connections on the given port and protocol.
    /// @pre http_server::request_received_event must have been called to register
    /// the request received callback function before this function.
    /// @throw logic_error if request_received_event has NOT been called
    /// before this function.
    /// @param port the port number to serve:
    /// default 80 for HTTP or 443 for HTTPS.
    /// @param ipv4_only whether an IPV4 only server is required, default false.
    /// @return the boost error code, false if no error occured
    ASIO_ERROR_CODE accept_connections
                      (unsigned short port = SocketAdaptor::DEFAULT_HTTP_PORT,
                       bool ipv4_only = false)
    {
      // If a request handler's not been registered, use the request_router
      if (!http_request_handler_)
        http_request_handler_ =
            [this](std::weak_ptr<http_connection_type> weak_ptr,
                   http::rx_request const& request, Container const& body)
        { route_request(weak_ptr, request, body); };

      return server_->accept_connections(port, ipv4_only);
    }

    /// Accessor for the request_router_
    request_router_type& request_router()
    { return request_router_; }

    ////////////////////////////////////////////////////////////////////////
    // Event Handlers

    /// Connect the request received callback function.
    ///
    /// If the application registers a handler for this event, then the
    /// application must determine how to respond to requests.
    /// Otherwise, the server will handle request with request_router_.
    /// @post disables the built-in request_router.
    /// @param handler the handler for a received HTTP request.
    void request_received_event(RequestHandler handler) NOEXCEPT
    { http_request_handler_ = handler; }

    /// Connect the chunk received callback function.
    /// @post disables automatic concatenating of chunks.
    /// @param handler the handler for a received HTTP chunk.
    void chunk_received_event(ChunkHandler handler) NOEXCEPT
    { http_chunk_handler_ = handler; }

    /// Connect the expect continue received callback function.
    ///
    /// If the application registers a handler for this event, then the
    /// application must determine how to respond to a request containing an
    /// Expect: 100-continue header based upon it's other headers.
    /// Otherwise, the server will automatically send a 100 Continue response,
    /// so that the client can continue to send the body of the request.
    /// @post disables automatic sending of a 100 Continue response
    /// @param handler the handler for an "expects continue" request.
    void request_expect_continue_event(RequestHandler handler) NOEXCEPT
    { http_continue_handler_ = handler; }

    /// Connect the invalid request received callback function.
    ///
    /// If the application registers a handler for this event, then the
    /// application must determine how to respond to invalid requests.
    /// @post disables automatic sending of an invalid response message.
    /// @post disables auto_disconnect_ (if enabled).
    /// @param handler the handler for a invalid request received.
    void invalid_request_event(RequestHandler handler) NOEXCEPT
    { http_invalid_handler_ = handler; }

    /// Connect the connected callback function.
    /// @param handler the handler for the socket connected event.
    void socket_connected_event(ConnectionHandler handler) NOEXCEPT
    { connected_handler_= handler; }

    /// Connect the disconnected callback function.
    /// @param handler the handler for the socket disconnected signal.
    void socket_disconnected_event(ConnectionHandler handler) NOEXCEPT
    { disconnected_handler_ = handler; }

    /// Connect the message sent callback function.
    /// @param handler the handler for the message sent signal.
    void message_sent_event(ConnectionHandler handler) NOEXCEPT
    { message_sent_handler_= handler; }

    ////////////////////////////////////////////////////////////////////////
    // HTTP Request Parser Parameter set functions

    /// Set whether to require strict CRLF HTTP request checking.
    /// @param enable strict CRLF HTTP checking
    void set_strict_crlf(bool enable) NOEXCEPT
    { strict_crlf_ = enable; }

    /// Set the maximum number of consecutive whitespace characters to allow.
    /// @param max_length default http_request::DEFAULT_MAX_WHITESPACE_CHARS.
    void set_max_whitespace(unsigned char max_length =
        http_request::DEFAULT_MAX_WHITESPACE_CHARS) NOEXCEPT
    { max_whitespace_ = max_length; }

    /// Set the maximum HTTP request method length to allow.
    /// @param max_length default http_request::DEFAULT_MAX_METHOD_LENGTH.
    void set_max_method_length(unsigned char max_length =
        http_request::DEFAULT_MAX_METHOD_LENGTH) NOEXCEPT
    { max_method_length_ = max_length; }

    /// Set the maximum HTTP request uri length to allow.
    /// @param max_length default http_request::DEFAULT_MAX_URI_LENGTH.
    void set_max_uri_length(size_t max_length =
        http_request::DEFAULT_MAX_URI_LENGTH) NOEXCEPT
    { max_uri_length_ = max_length; }

    /// Set the maximum HTTP request header line length to allow.
    /// @param max_length default http_request::DEFAULT_MAX_LINE_LENGTH.
    void set_max_header_line_length(unsigned short max_length =
        http_request::DEFAULT_MAX_LINE_LENGTH) NOEXCEPT
    { max_line_length_ = max_length; }

    /// Set the maximum number of HTTP request header fields to allow.
    /// @param max_number default http_request::DEFAULT_MAX_HEADER_NUMBER.
    void set_max_number_of_headers(unsigned short max_number =
        http_request::DEFAULT_MAX_HEADER_NUMBER) NOEXCEPT
    { max_header_number_ = max_number; }

    /// Set the maximum total length of HTTP request headers to allow.
    /// @param max_length default http_request::DEFAULT_MAX_HEADER_LENGTH.
    void set_max_headers_length(size_t max_length =
        http_request::DEFAULT_MAX_HEADER_LENGTH) NOEXCEPT
    { max_header_length_ = max_length; }

    /// Set the maximum HTTP request body size to allow.
    /// @param max_size default http_request::DEFAULT_MAX_BODY_SIZE.
    void set_max_body_size(size_t max_size =
        http_request::DEFAULT_MAX_BODY_SIZE) NOEXCEPT
    { max_body_size_ = max_size; }

    /// Set the maximum HTTP request chunk size to allow.
    /// @param max_size default http_request::DEFAULT_MAX_CHUNK_SIZE.
    void set_max_chunk_size(size_t max_size =
        http_request::DEFAULT_MAX_CHUNK_SIZE) NOEXCEPT
    { max_chunk_size_ = max_size; }

    ////////////////////////////////////////////////////////////////////////
    // HTTP server options set functions

    /// Enable whether the http server translates HEAD requests into GET
    /// requests for the application.
    /// Note: http_server never sends a body in a response to a HEAD request.
    /// @post HEAD translation enabled/disabled.
    /// @param enable enable the function, default true.
    void set_translate_head(bool enable = true) NOEXCEPT
    { translate_head_ = enable; }

    /// Enable whether the http server echos TRACE requests.
    ///
    /// The standard HTTP response to a TRACE request is to echo back the
    /// TRACE message and all of it's headers in the body of the response.
    /// However it's considered a security vulnerability nowadays, so the
    /// default behaviour is to send a 405 "Method Not Allowed" response instead.
    /// @param enable enable the function, default false.
    void set_trace_enabled(bool enable = false) NOEXCEPT
    { trace_enabled_ = enable; }

    /// Enable whether the http server automatically disconnects invalid requests.
    /// @pre if invalid_request_handler is called to register an application
    /// handler for invlaid requests then auto_disconnect_ is ignored.
    /// @param enable enable the function, default false.
    void set_auto_disconnect(bool enable = false) NOEXCEPT
    { auto_disconnect_ = enable; }

    /// Set the size of the server receive buffer.
    /// @param size the new size of the receive buffer, default
    /// SocketAdaptor::DEFAULT_RX_BUFFER_SIZE
    void set_rx_buffer_size(size_t size = SocketAdaptor::DEFAULT_RX_BUFFER_SIZE) NOEXCEPT
    { server_->set_rx_buffer_size(size); }

    /// Set the tcp keep alive status for all future connections.
    /// @param enable if true enables the tcp socket keep alive status.
    void set_keep_alive(bool enable) NOEXCEPT
    { server_->set_keep_alive(enable); }

    /// Set the send and receive timeout values for all future connections.
    /// @pre sockets may remain open forever
    /// @post sockets will close if no activity has occured after the
    /// timeout period.
    /// @param timeout the timeout in milliseconds.
    void set_timeout(int timeout) NOEXCEPT
    { server_->set_timeout(timeout); }

    ////////////////////////////////////////////////////////////////////////
    // HTTPS set functions

    /// Set the password for an SSL connection.
    /// @pre http_server derived from via::comms::ssl::ssl_tcp_adaptor.
    /// @param password the SSL password
    void set_password(std::string const& password) NOEXCEPT
    { server_->set_password(password); }

    /// Set the files required for an SSL server.
    /// @pre http_server derived from via::comms::ssl::ssl_tcp_adaptor.
    /// @param certificate_file the server SSL certificate file.
    /// @param key_file the private key file
    /// @param dh_file the dh file.
    static ASIO_ERROR_CODE set_ssl_files
                       (const std::string& certificate_file,
                        const std::string& key_file,
                        std::string        dh_file = "")
    {
      ASIO_ERROR_CODE error;
#ifdef HTTP_SSL
      server_type::connection_type::ssl_context().
          use_certificate_file(certificate_file,
                               ASIO::ssl::context::pem, error);
      if (error)
        return error;

      server_type::connection_type::ssl_context().
          use_private_key_file(key_file, ASIO::ssl::context::pem,
                               error);
      if (error)
        return error;

      if (dh_file.empty())
        server_type::connection_type::ssl_context().
           set_options(ASIO::ssl::context::default_workarounds |
                       ASIO::ssl::context::no_sslv2);
      else
      {
        server_type::connection_type::ssl_context().use_tmp_dh_file(dh_file,
                                                                   error);
        if (error)
          return error;

        server_type::connection_type::ssl_context().
           set_options(ASIO::ssl::context::default_workarounds |
                       ASIO::ssl::context::no_sslv2 |
                       ASIO::ssl::context::single_dh_use,
                       error);
      }
#endif // HTTP_SSL
      return error;
    }

    ////////////////////////////////////////////////////////////////////////
    // other functions

    /// Disconnect all of the outstanding http server connections to prepare
    /// for closing the server.
    void shutdown()
    {
      if (!shutting_down_ && !http_connections_.empty())
      {
        shutting_down_ = true;
        for (auto& elem : http_connections_)
          elem.second->disconnect();
        //for (auto item = http_connections_.begin(); item!= http_connections_.end(); ++item)
        //  item->second->disconnect();
        http_connections_.clear();
      }
      else
        close();
    }

    /// Close the http server and all of the connections associated with it.
    void close()
    {
      http_connections_.clear();
      server_->close();
    }

    /// Accessor function for the comms server.
    /// @return a shared pointer to the server
    std::shared_ptr<server_type> tcp_server() NOEXCEPT
    { return server_; }
  };

}
#endif
