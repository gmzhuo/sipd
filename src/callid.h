#pragma once
#include <memory>
#include <map>
#include <string>
#include <boost/asio.hpp>


class SIPEndpoint;
class SIPEndpoint;

class callID{
public:
	callID(boost::asio::io_context &context, const char *id, SIPEndpoint *ep);
	virtual ~callID() = default;
public:
	void onConfirmed();
	void onUpdate();
protected:
	void onExpired();
protected:
	std::string m_ID;
	SIPEndpoint *m_ep;
	boost::asio::steady_timer m_timer;
};
