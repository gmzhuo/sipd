#include <iostream>

#include "siprouter.h"
#include "sipendpoint.h"
#include "sipmessage.h"

SIPRouter::SIPRouter(const std::string realm):
  m_realm(realm)
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

	std::cout << "Loged Call-ID: " << callID << " for UA " << from->getUA() <<std::endl;

	auto it = m_endpointsMapByCallID.find(from->getUA());
	if(it != m_endpointsMapByCallID.end()) {
		it->second[callID] = from;
	} else {
		std::map<std::string, std::weak_ptr<SIPEndpoint>> epm;
		epm[callID] = from;
		m_endpointsMapByCallID[from->getUA()] = epm;
		std::cout << "Record one callID: " << callID << " for UA: " << from->getUA() << std::endl;
	}


	auto target = message->getTarget();

	auto eps = m_endpoints.find(target);
	if(eps != m_endpoints.end()) {
		for(auto it = eps->second.begin(); it != eps->second.end(); ++it) {
			auto &dest = it->second;
			dest->sendMessage(message);
		}
	}
}


void SIPRouter::forwardStatus(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message)
{
	std::string ua;

	
	auto &head = message->getHeader();
	auto callID = head["Call-ID"];

	std::cout << "Call-ID: " << callID <<std::endl;

	auto it = m_endpointsMapByCallID.find(ua);

	if(it == m_endpointsMapByCallID.end()) {
		return;
	} else {
		auto &epm = it->second;
		auto itep = epm.find(callID);
		if(itep != epm.end()) {
			auto destep = itep->second.lock();
			if(destep) {
				destep->sendMessage(message);
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
	std::cout << "in SIPRouter::registerEndpoint " << ep->getUA() << std::endl;
	if(!ep)
		return;

	auto ua = ep->getUA();
	if(ua.length() <= 0)
		return;

	std::cout << "in SIPRouter::registerEndpoint store ep " << ep->getUA() << std::endl;
	auto it = m_endpoints.find(ua);
	if(it != m_endpoints.end()) {
		it->second[ep.get()] = ep;
	} else {
		std::map<void *, std::shared_ptr<SIPEndpoint>> epm;
		epm[ep.get()] = (ep);
		m_endpoints[ua] = epm;
	}
}
