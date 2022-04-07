//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//
//------------------------------------------------------------------------------
//
// Example: WebSocket server, synchronous
//
//------------------------------------------------------------------------------
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <codecvt>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using stream = boost::asio::local::stream_protocol;       // from <boost/local/ip/tcp.stream_protocol>


class session
	: public std::enable_shared_from_this<session>
{
public:
	session(tcp::socket socket)
		: ws(std::move(socket))
	{
	}

	void start()
	{
		// Set suggested timeout settings for the websocket
		ws.set_option(
			websocket::stream_base::timeout::suggested(
			beast::role_type::server));

        // Set a decorator to change the Server of the handshake
		ws.set_option(websocket::stream_base::decorator(
			[](websocket::response_type& res)
			{
				res.set(http::field::server,
					std::string(BOOST_BEAST_VERSION_STRING) +
					" websocket-server-async");
            }));

		// Accept the websocket handshake
		ws.async_accept(
			beast::bind_front_handler(
			&session::on_accept,
			shared_from_this()));
	}

private:
	void
    on_accept(beast::error_code ec)
    {
		if(ec)
			return;

		// Read a message
		do_read();
	}

	void do_read()
	{
		// Read a message into our buffer
		ws.async_read(
			buffer_, beast::bind_front_handler(
				&session::on_read,
				shared_from_this()));
	}

	void
	on_read(beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		// This indicates that the session was closed
		if(ec == websocket::error::closed)
			return;

		if(ec)
			return;

		// Echo the message
		ws.text(ws.got_text());
		ws.async_write(
			buffer_.data(),
			beast::bind_front_handler(
				&session::on_write,
				shared_from_this()));
	}

	void
	on_write(
		beast::error_code ec,
		std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if(ec)
			return;

		// Clear the buffer
		buffer_.consume(buffer_.size());

		// Do another read
		do_read();
	}

	websocket::stream<tcp::socket> ws;
	beast::flat_buffer buffer_;
};

class server
{
public:
	server(boost::asio::io_context& io_context, const char *pathname, unsigned short port)
	 : acceptor_(io_context, tcp::endpoint(net::ip::make_address(pathname), port))
	{
		do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(
			[this](boost::system::error_code ec, tcp::socket socket)
			{
				if (!ec)
				{
					std::make_shared<session>(std::move(socket))->start();
				}

				do_accept();
			});
	}

	tcp::acceptor acceptor_;
};


void run(const char *pathname, unsigned short port)
{
	boost::asio::io_context io_context;

	server s(io_context, pathname, port);

	io_context.run();
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    try
    {
		run(argv[1], (unsigned short)atoi(argv[2]));
    }
    catch (const std::exception & e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}