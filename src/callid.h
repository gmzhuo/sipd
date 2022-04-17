#pragma once
#include <memory>
#include <map>
#include <string>
#include <boost/asio.hpp>


class SIPEndpoint;
class SIPRouter;

class callID{
public:
	callID(boost::asio::io_context &context, const char *id, SIPRouter *router);
	virtual ~callID() = default;
public:
	void addEndpoint(const std::shared_ptr<SIPEndpoint> ep);
	void removeEndpoint(const char *ua);
	std::shared_ptr<SIPEndpoint> findEndpoint(const char *ua);
	void onConfirmed();
	void onUpdate();
protected:
	void onExpired();
protected:
	std::map<std::string, std::weak_ptr<SIPEndpoint>> m_eps;
	std::string m_ID;
	SIPRouter *m_router;
	boost::asio::steady_timer m_timer;
};