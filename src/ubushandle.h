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
#include <core/servdcontext.h>

class descriptor_wait_op: public boost::asio::detail::reactor_op
{
public:
	descriptor_wait_op(ubusContext* context, const boost::system::error_code& success_ec,
		func_type complete_func)
		:reactor_op(success_ec,&descriptor_wait_op::do_perform, complete_func), m_context(context)
	{
	}

	static status do_perform(reactor_op* base)
	{
		descriptor_wait_op *op = (descriptor_wait_op *)base;
		if(op->m_context) {
			op->m_context->handleEvent();
		}
		boost::asio::detail::reactor_op::status result = (boost::asio::detail::reactor_op::status)0;
		return result;
	}
protected:
	ubusContext *m_context;
};

class ubusASIOContextHandler
{
public:
	ubusASIOContextHandler(boost::asio::io_context& context);
public:
	void waitUBUSHandleEventAsync();
protected:
	static void handleEventForCommonContext(void*, boost::asio::detail::scheduler_operation*,
			const boost::system::error_code&, long unsigned int);
	static void handleEventForServdContext(void*, boost::asio::detail::scheduler_operation*,
			const boost::system::error_code&, long unsigned int);
protected:
	boost::asio::io_context& m_context;
	boost::asio::detail::reactor& reactor_;
	int m_ubus_handle_global;
	int m_ubus_handle_server;
	boost::asio::detail::reactor::per_descriptor_data m_global_data;
	boost::asio::detail::reactor::per_descriptor_data m_server_data;
	boost::system::error_code g_ecode;
	boost::system::error_code s_ecode;
	descriptor_wait_op m_gop;
	descriptor_wait_op m_sop;
};