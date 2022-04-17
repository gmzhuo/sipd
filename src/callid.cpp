#include "callid.h"
#include "sipendpoint.h"
#include "siprouter.h"


callID::callID(boost::asio::io_context &context, const char *id, SIPRouter *router):
	m_ID(id), m_router(router), m_timer(context)
{
	auto callIDExpired = [this] (const boost::system::error_code& /*e*/){
		this->onExpired();
	};

	m_timer.expires_at(m_timer.expiry() + boost::asio::chrono::seconds(40));
	m_timer.async_wait(callIDExpired);
}

void callID::addEndpoint(const std::shared_ptr<SIPEndpoint> ep)
{
	if(ep) {
		m_eps[ep->getUA()] = ep;
	}
}

void callID::removeEndpoint(const char *ua)
{
	if(ua) {
		auto it = m_eps.find(ua);
		if(it != m_eps.end()) {
			m_eps.erase(it);
		}
	}
}

std::shared_ptr<SIPEndpoint> callID::findEndpoint(const char *ua)
{
	static std::shared_ptr<SIPEndpoint> invalid;

	auto callIDExpired = [this] (const boost::system::error_code& /*e*/){
		this->onExpired();
	};

	if(ua) {
		auto it = m_eps.find(ua);
		if(it != m_eps.end()) {
			auto ep = it->second.lock();
			if(!ep) {
				//set expired after 1s
				m_timer.cancel();
				m_timer.expires_at(m_timer.expiry() + boost::asio::chrono::seconds(1));
				m_timer.async_wait(callIDExpired);
			}
			return ep;
		}
	}

	return invalid;
}

void callID::onExpired()
{
	m_router->removeCallID(m_ID);
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
	m_timer.expires_at(m_timer.expiry() + boost::asio::chrono::seconds(1));
	m_timer.async_wait(callIDExpired);
}
