#include <iostream>

#include "siprouter.h"
#include "sipendpoint.h"
#include "sipmessage.h"
#include "callid.h"
#include "sqlite3.h"

SIPRouter::SIPRouter(boost::asio::io_context &context, const std::string realm):
  m_realm(realm), m_context(context)
{
}

void SIPRouter::onEndpointClosed(std::shared_ptr<SIPEndpoint>& ep)
{
	if(!ep)
		return;

	auto ua = ep->getUA();
	if(ua.length() <= 0)
		return;

	//do some cleaning
	auto it = m_endpoints.find(ua);
	if(it != m_endpoints.end()) {
		auto &eps = it->second;
		auto epit = eps.find(ep.get());
		if(epit != eps.end()) {
			std::cout << "in SIPRouter::onSessionClosed remve ep " << ep->getUA() << std::endl;
			eps.erase(epit);
		}
	}
}

void SIPRouter::forwardMessage(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message)
{
	auto &head = message->getHeader();
	auto callID = head["Call-ID"];

	std::const_pointer_cast<SIPEndpoint>(from)->addCallID(callID);

	auto target = message->getTarget();

	auto eps = m_endpoints.find(target);
	if(eps != m_endpoints.end()) {
		for(auto it = eps->second.begin(); it != eps->second.end(); ++it) {
			auto &dest = it->second;
			dest->addCallID(callID);
			dest->forwardMessage(message);
		}
	}

	auto devices = sqlite3DB::getInstance().getUserDevice(target.c_str());

	for(auto it = devices.begin(); it != devices.end(); ++it) {
		//based on the device id to wakeup the device.
	}
}


void SIPRouter::forwardStatus(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message)
{
	std::string target = message->getTarget();

	auto &head = message->getHeader();
	auto callID = head["Call-ID"];

	std::const_pointer_cast<SIPEndpoint>(from)->addCallID(callID);

	auto eps = m_endpoints.find(target);
	if(eps != m_endpoints.end()) {
		for(auto it = eps->second.begin(); it != eps->second.end(); ++it) {
			auto &dest = it->second;
			printf("check call ID %s\r\n", callID.c_str());
			if(dest->haveCallID(callID)) {
				dest->forwardMessage(message);
			} else {
				printf("check call ID %s failed for target %s\r\n", callID.c_str(), target.c_str());
			}
		}
	}
}

void SIPRouter::forwardOutput(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message)
{
	//create or find the connection to remote server;
	std::shared_ptr<SIPEndpoint> remoteServer;

	if(remoteServer) {
		remoteServer->sendMessage(message);
	}
}

void SIPRouter::registerEndpoint(std::shared_ptr<SIPEndpoint> &ep)
{
	if(!ep)
		return;

	auto ua = ep->getUA();
	if(ua.length() <= 0)
		return;

	auto it = m_endpoints.find(ua);
	if(it != m_endpoints.end()) {
		it->second[ep.get()] = ep;
	} else {
		std::map<void *, std::shared_ptr<SIPEndpoint>> epm;
		epm[ep.get()] = (ep);
		m_endpoints[ua] = epm;
	}
}


