#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/ssl.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <codecvt>

#include "sipendpoint.h"
#include "sipmessage.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using stream = boost::asio::local::stream_protocol;       // from <boost/local/ip/tcp.stream_protocol>

class wsSession;

class wsEndpoint:public SIPEndpoint {
public:
	wsEndpoint(boost::asio::io_context &context, SIPRouter *router, wsSession* session)
		:SIPEndpoint(context, router), m_wsSession(session)
	{
	}
protected:
	virtual int sendMessage(const std::shared_ptr<SIPMessage>& message);
protected:
	wsSession *m_wsSession;
};

class wsSession
	: public std::enable_shared_from_this<wsSession>
{
public:
	wsSession(boost::asio::io_context& context, SIPRouter *router, tcp::socket socket)
		: ws(std::move(socket)), m_context(context)
	{
		m_endpoint = std::make_shared<wsEndpoint>(m_context, router, this);
	}
	virtual ~wsSession()
	{
		if(m_endpoint) {
			m_endpoint->onEndpointClosed();
		}
	}
public:
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
			&wsSession::on_accept,
			shared_from_this()));
	}
public:
	void sendMessage(const char *msg, size_t len)
	{
		ws.write(boost::asio::buffer(msg, len));
		//	[this](beast::error_code ec, std::size_t bytes_transferred)
		//	{
		//	});
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
				&wsSession::on_read,
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

		auto data = buffer_.data();
		if(m_endpoint) {
			m_endpoint->onBuffer((const char *)data.data(), data.size());
		}

		buffer_.consume(data.size());
		do_read();
	}

	void
	on_write(
		beast::error_code ec,
		std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if(ec)
			return;

		buffer_.consume(buffer_.size());
	}

	websocket::stream<tcp::socket> ws;
	beast::flat_buffer buffer_;
	std::shared_ptr<wsEndpoint> m_endpoint;
	boost::asio::io_context& m_context;
};

class wsServer
{
public:
	wsServer(SIPRouter *router, boost::asio::io_context& io_context, const char *pathname, unsigned short port)
	 : acceptor_(io_context, tcp::endpoint(net::ip::make_address(pathname), port)), m_router(router),
	 m_context(io_context)
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
					std::make_shared<wsSession>(m_context, m_router, std::move(socket))->start();
				}

				do_accept();
			});
	}

	tcp::acceptor acceptor_;
	SIPRouter *m_router;
	boost::asio::io_context& m_context;
};

