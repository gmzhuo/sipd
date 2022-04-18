#pragma once
#include <string>
#include <memory>
#include <map>
#include <list>
#include <boost/asio.hpp>

class SIPEndpoint;
class SIPMessage;
class callID;

class SIPRouter{
public:
	SIPRouter(boost::asio::io_context &context, const std::string realm);
	virtual ~SIPRouter() = default;
	SIPRouter(const SIPRouter& router) = delete;
	SIPRouter& operator=(const SIPRouter& router) = delete;
protected:
	void forwardOutput(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message);
public:
	const std::string& getRealm() const
	{
		return m_realm;
	}
	void onSessionClosed(std::shared_ptr<SIPEndpoint>& ep);
	void removeCallID(const std::string& id);
public:
	void forwardMessage(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message);
	void forwardStatus(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message);
	void registerEndpoint(std::shared_ptr<SIPEndpoint>& ep);
protected:
	std::shared_ptr<SIPEndpoint> findEndpointByCallID(const char *callID, const char *ua);
	void addEndpointIntoCallID(const char *id, const std::shared_ptr<SIPEndpoint>& from);
private:
	std::map<std::string, std::map<void *, std::shared_ptr<SIPEndpoint>>> m_endpoints;
	std::map<std::string, std::shared_ptr<callID>> m_callIDs;
	std::string m_realm;
	boost::asio::io_context &m_context;
};
