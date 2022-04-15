#include "ubushandle.h"
#include <core/servdcontext.h>


void ubusASIOContextHandler::waitUBUSHandleEvent()
{
	boost::asio::detail::reactor_op *op;

	auto handleEventForCommonContext = [] (void*, boost::asio::detail::scheduler_operation*,
			const boost::system::error_code&, long unsigned int){
		ubusContext::getCommonContext().handleEvent();
	};

	auto handleEventForServdContext = [] (void*, boost::asio::detail::scheduler_operation*,
			const boost::system::error_code&, long unsigned int){
		servdContext::getInstance().handleEvent();
	};

	static boost::system::error_code g_ecode;
	static boost::system::error_code s_ecode;


	descriptor_wait_op gop(g_ecode, handleEventForCommonContext);
	descriptor_wait_op sop(s_ecode, handleEventForServdContext);

	reactor_.start_op(boost::asio::detail::reactor::read_op, m_ubus_handle_global, m_global_data, &gop, false, false);
	reactor_.start_op(boost::asio::detail::reactor::read_op, m_ubus_handle_server, m_server_data, &sop, false, false);
}