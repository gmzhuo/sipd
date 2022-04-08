#include <iostream>
#include <sstream>
#include "sipendpoint.h"
#include "sipmessage.h"

SIPEndpoint::SIPEndpoint(SIPRouter *router):
	m_router(router)
{
}

SIPEndpoint::~SIPEndpoint()
{
	printf("endpoint destructed\r\n");
}
	
void SIPEndpoint::onSessionClosed()
{
	printf("in end point on closed %p\r\n", this);

	printf("in end point on closed call router\r\n");
	if(m_myself && m_router) {
		m_router->onSessionClosed(m_myself);
	}
	printf("in end point on closed call router done\r\n");
}

void SIPEndpoint::onMessage(const std::shared_ptr<SIPMessage>& message)
{
	auto type = message->getTypeString();
	std::cout << "on message " << type << std::endl;
	switch(message->getType()) {
	case sipRegister:
		doRegister(message);
		break;
	default:
		break;
	}
}

void SIPEndpoint::doRegister(const std::shared_ptr<SIPMessage>& message)
{
	std::ostringstream os;

	auto resp = message->makeResponse(200, "OK", "", "");

	this->sendMessage(resp);
}