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

class sslSession;

class sslEndpoint:public SIPEndpoint {
public:
	sslEndpoint(boost::asio::io_context &context, SIPRouter *router, sslSession* session)
		:SIPEndpoint(context, router), m_sslSession(session)
	{
	}
protected:
	virtual int sendMessage(const std::shared_ptr<SIPMessage>& message);
protected:
	sslSession *m_sslSession;
};

class sslSession : public std::enable_shared_from_this<sslSession>
{
public:
	sslSession(boost::asio::io_context& context, SIPRouter *router, boost::asio::ssl::stream<tcp::socket> socket)
		: socket_(std::move(socket)), m_context(context)
	{
		m_endpoint = std::make_shared<sslEndpoint>(m_context, router, this);
	}

	virtual ~sslSession()
	{
		if(m_endpoint) {
			m_endpoint->onEndpointClosed();
		}
	}

public:
	void start()
	{
		do_handshake();
	}
public:
	void sendMessage(const char *msg, size_t len)
	{
		boost::asio::async_write(socket_, boost::asio::buffer(msg, len),
			[this](const boost::system::error_code& ec,
				std::size_t /*length*/)
			{
			}
		);
	}
private:
	void do_handshake()
	{
		auto self(shared_from_this());
		socket_.async_handshake(boost::asio::ssl::stream_base::server, 
			[this, self](const boost::system::error_code& error)
			{
				if (!error)
				{
					do_read();
				}
			}
		);
	}

	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_),
			[this, self](const boost::system::error_code& ec, std::size_t length)
			{
				if (!ec)
				{
					data_[length] = 0;
					if(length == 4) {
						sendMessage(data_, 4);
					} else {
						//auto sm = std::make_shared<SIPMessage>(data_, length);
						if(m_endpoint) {
							m_endpoint->onBuffer(data_, length);
						}
					}
					do_read();
				}
			}
		);
	}

	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
			[this, self](const boost::system::error_code& ec,
				std::size_t /*length*/)
			{
			}
		);
	}

	boost::asio::ssl::stream<tcp::socket> socket_;
	char data_[2048 * 16];
	std::shared_ptr<sslEndpoint> m_endpoint;
	boost::asio::io_context& m_context;
};

class sslServer
{
public:
	sslServer(SIPRouter *router, boost::asio::io_context& io_context, unsigned short port)
		: acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
		context_(boost::asio::ssl::context::sslv23),
		m_router(router), m_context(io_context)
	{
		context_.set_options(
			boost::asio::ssl::context::default_workarounds
				| boost::asio::ssl::context::no_sslv2
				| boost::asio::ssl::context::single_dh_use);
		context_.set_password_callback(std::bind(&sslServer::get_password, this));
		context_.use_certificate_chain_file("/etc/nginx/server.crt");
		context_.use_private_key_file("/etc/nginx/server.key.unsecure", boost::asio::ssl::context::pem);
		context_.use_tmp_dh_file("dh2048.pem");

		do_accept();
	}

private:
	std::string get_password() const
	{
		return "test";
	}

	void do_accept()
	{
		acceptor_.async_accept(
			[this](const boost::system::error_code& error, tcp::socket socket)
			{
				if (!error)
				{
					std::make_shared<sslSession>(
						m_context, m_router,
						boost::asio::ssl::stream<tcp::socket>(
							std::move(socket), context_))->start();
				}

				do_accept();
			});
	}

	tcp::acceptor acceptor_;
	boost::asio::ssl::context context_;
	SIPRouter *m_router;
	boost::asio::io_context& m_context;
};
