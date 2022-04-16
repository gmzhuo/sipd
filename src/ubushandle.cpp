#include "ubushandle.h"

ubusASIOContextHandler::ubusASIOContextHandler(boost::asio::io_context& context):
	m_context(context), reactor_(boost::asio::use_service<boost::asio::detail::reactor>(context)),
	m_gop(&ubusContext::getCommonContext(), g_ecode, handleEventForCommonContext),
	m_sop(&servdContext::getInstance(), s_ecode, handleEventForServdContext)
{
	auto gContext = ubusContext::getCommonContext().getContext();
	auto sContext = servdContext::getInstance().getContext();

	unsigned int fl;
	fl = fcntl(gContext->sock.fd, F_GETFL, 0);
	fl |= O_NONBLOCK;
	fcntl(gContext->sock.fd, F_SETFL, fl);

	fl = fcntl(sContext->sock.fd, F_GETFL, 0);
	fl |= O_NONBLOCK;
	fcntl(sContext->sock.fd, F_SETFL, fl);

	m_ubus_handle_global = gContext->sock.fd;
	m_ubus_handle_server = sContext->sock.fd;

	reactor_.register_descriptor(m_ubus_handle_global, m_global_data);
	reactor_.register_descriptor(m_ubus_handle_server, m_server_data);
}

void ubusASIOContextHandler::handleEventForCommonContext(void* notuse, boost::asio::detail::scheduler_operation*,
		const boost::system::error_code&, long unsigned int)
{
}

void ubusASIOContextHandler::handleEventForServdContext(void* THIS, boost::asio::detail::scheduler_operation*,
	const boost::system::error_code&, long unsigned int)
{
};

void ubusASIOContextHandler::waitUBUSHandleEventAsync()
{
	reactor_.start_op(boost::asio::detail::reactor::read_op, m_ubus_handle_global,
			m_global_data, &m_gop, true, false);

	reactor_.start_op(boost::asio::detail::reactor::read_op, m_ubus_handle_server,
			m_server_data, &m_sop, true, false);
}