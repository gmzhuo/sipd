#include "callid.h"
#include "sipendpoint.h"


callID::callID(boost::asio::io_context &context, const char *id, SIPEndpoint *ep):
	m_ID(id), m_ep(ep), m_timer(context)
{
	auto callIDExpired = [this] (const boost::system::error_code& /*e*/){
		this->onExpired();
	};

	m_timer.expires_at(m_timer.expiry() + boost::asio::chrono::seconds(40000));
	m_timer.async_wait(callIDExpired);
}

void callID::onExpired()
{
	//m_ep->removeCallID(m_ID);
}

void callID::onConfirmed()
{
	//todo
	//we should remove timeout or set longer expired timer
	m_timer.cancel();
}

void callID::onUpdate()
{
	auto callIDExpired = [this] (const boost::system::error_code& /*e*/){
		this->onExpired();
	};

	m_timer.cancel();
	m_timer.expires_at(m_timer.expiry() + boost::asio::chrono::seconds(4000000));
	m_timer.async_wait(callIDExpired);
}
