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

void SIPRouter::onSessionClosed(std::shared_ptr<SIPEndpoint>& ep)
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

	addEndpointIntoCallID(callID.c_str(), from);

	auto target = message->getTarget();

	auto eps = m_endpoints.find(target);
	if(eps != m_endpoints.end()) {
		for(auto it = eps->second.begin(); it != eps->second.end(); ++it) {
			auto &dest = it->second;
			dest->sendMessage(message);
		}
	}

	auto devices = sqlite3DB::getInstance().getUserDevice(target.c_str());

	for(auto it = devices.begin(); it != devices.end(); ++it) {
		//based on the device id to wakeup the device.
	}
}


void SIPRouter::forwardStatus(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message)
{
	std::string ua = message->getTarget();

	
	auto &head = message->getHeader();
	auto callID = head["Call-ID"];

	addEndpointIntoCallID(callID.c_str(), from);

	auto toep = findEndpointByCallID(callID.c_str(), ua.c_str());

	if(toep) {
		toep->sendMessage(message);
	}

	//todo
	//We should monitor the callID status by the message status.
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

void SIPRouter::removeCallID(const std::string& id)
{
	auto it = m_callIDs.find(id);

	if(it != m_callIDs.end()) {
		m_callIDs.erase(it);
	}
}

std::shared_ptr<SIPEndpoint> SIPRouter::findEndpointByCallID(const char *id, const char *ua)
{
	std::shared_ptr<SIPEndpoint> invalid;

	auto it = m_callIDs.find(id);
	if(it == m_callIDs.end()) {
		return invalid;
	}

	auto &pid = it->second;
	if(pid)
		return pid->findEndpoint(ua);

	return invalid;
}

void SIPRouter::addEndpointIntoCallID(const char *id, const std::shared_ptr<SIPEndpoint>& from)
{
	auto it = m_callIDs.find(id);
	if(it == m_callIDs.end()) {
		auto ptr = std::make_shared<callID>(m_context, id, this);
		if(ptr) {
			ptr->addEndpoint(from);
			m_callIDs[id] = ptr;
		}
	} else {
		it->second->addEndpoint(from);
	}
}

