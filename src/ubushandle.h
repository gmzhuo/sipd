#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/detail/reactive_null_buffers_op.hpp>
#include <boost/asio/detail/reactive_wait_op.hpp>
#include <boost/asio/detail/reactor.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <codecvt>

class descriptor_wait_op: public boost::asio::detail::reactor_op
{
public:
  descriptor_wait_op(const boost::system::error_code& success_ec,
      func_type complete_func)
    : reactor_op(success_ec,
        &descriptor_wait_op::do_perform, complete_func)
  {
  }

  static status do_perform(reactor_op* base)
  {
    boost::asio::detail::reactor_op::status result = (boost::asio::detail::reactor_op::status)0;
    return result;
  }
};

class ubusASIOContextHandler
{
public:
	ubusASIOContextHandler(boost::asio::io_context& context):
		m_context(context), reactor_(boost::asio::use_service<boost::asio::detail::reactor>(context))
	{
	}
public:
	void waitUBUSHandleEvent();
protected:
	boost::asio::io_context& m_context;
	boost::asio::detail::reactor& reactor_;
	int m_ubus_handle_global;
	int m_ubus_handle_server;
	boost::asio::detail::reactor::per_descriptor_data m_global_data;
	boost::asio::detail::reactor::per_descriptor_data m_server_data;
};