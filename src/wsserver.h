#include <websocketpp/common/asio.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/functional.hpp>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace asio = websocketpp::lib::asio;

struct tcp_echo_session : websocketpp::lib::enable_shared_from_this<tcp_echo_session> {
    typedef websocketpp::lib::shared_ptr<tcp_echo_session> ptr;
    
    tcp_echo_session(asio::io_service & service) : m_socket(service) {}

    void start() {
        m_socket.async_read_some(asio::buffer(m_buffer, sizeof(m_buffer)),
            websocketpp::lib::bind(
                &tcp_echo_session::handle_read, shared_from_this(), _1, _2));
    }
    
    void handle_read(const asio::error_code & ec, size_t transferred) {
        if (!ec) {
            asio::async_write(m_socket,
                asio::buffer(m_buffer, transferred),
                    bind(&tcp_echo_session::handle_write, shared_from_this(), _1));
        }
    }
    
    void handle_write(const asio::error_code & ec) {
        if (!ec) {
            m_socket.async_read_some(asio::buffer(m_buffer, sizeof(m_buffer)),
                bind(&tcp_echo_session::handle_read, shared_from_this(), _1, _2));
        }
    }

    asio::ip::tcp::socket m_socket;
    char m_buffer[1024];
};

struct local_echo_session : websocketpp::lib::enable_shared_from_this<local_echo_session> {
    typedef websocketpp::lib::shared_ptr<local_echo_session> ptr;
    
    local_echo_session(asio::io_service & service) : m_socket(service) {}

    void start() {
        m_socket.async_read_some(asio::buffer(m_buffer, sizeof(m_buffer)),
            websocketpp::lib::bind(
                &local_echo_session::handle_read, shared_from_this(), _1, _2));
    }
    
    void handle_read(const asio::error_code & ec, size_t transferred) {
        if (!ec) {
            asio::async_write(m_socket,
                asio::buffer(m_buffer, transferred),
                    bind(&local_echo_session::handle_write, shared_from_this(), _1));
        }
    }
    
    void handle_write(const asio::error_code & ec) {
        if (!ec) {
            m_socket.async_read_some(asio::buffer(m_buffer, sizeof(m_buffer)),
                bind(&local_echo_session::handle_read, shared_from_this(), _1, _2));
        }
    }

    asio::local::stream_protocol::socket m_socket;
    char m_buffer[2048];
};

struct tcp_echo_server {
    tcp_echo_server(asio::io_service & service, short port)
        : m_service(service)
        , m_acceptor(service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port))
    {
        this->start_accept();
    }
    
    void start_accept() {
        tcp_echo_session::ptr new_session(new tcp_echo_session(m_service));
        m_acceptor.async_accept(new_session->m_socket,
            bind(&tcp_echo_server::handle_accept, this, new_session, _1));
    }
    
    void handle_accept(tcp_echo_session::ptr new_session, const asio::error_code & ec) {
        if (!ec) {
            new_session->start();
        }
        start_accept();
    }

    asio::io_service & m_service;
    asio::ip::tcp::acceptor m_acceptor;
};


struct local_echo_server {
	local_echo_server(asio::io_service & service, std::string address):
		m_service(service),
		m_acceptor(m_service, asio::local::stream_protocol::endpoint(address))
	{
		this->start_accept();
	}

	void start_accept() {
        local_echo_session::ptr new_session(new local_echo_session(m_service));
        m_acceptor.async_accept(new_session->m_socket,
            bind(&local_echo_server::handle_accept, this, new_session, _1));
    }

	void handle_accept(local_echo_session::ptr new_session, const asio::error_code & ec) {
        if (!ec) {
            new_session->start();
        }
        start_accept();
    }

	asio::io_service & m_service;
	asio::local::stream_protocol::acceptor m_acceptor;
};

