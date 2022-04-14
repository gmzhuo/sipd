#include <iostream>
#include <sstream>
#include <mutex>
#include "sipendpoint.h"
#include "sipmessage.h"

SIPEndpoint::SIPEndpoint(SIPRouter *router):
	m_router(router)
{
}

SIPEndpoint::~SIPEndpoint()
{
}

void SIPEndpoint::onSessionClosed()
{
	auto THIS = shared_from_this();
	m_router->onSessionClosed(THIS);
}

void SIPEndpoint::onMessage(const std::shared_ptr<SIPMessage>& message)
{
	auto type = message->getTypeString();

	printf("ep %s recive:\r\n%s", m_ua.c_str(), message->getMessage().c_str());
	auto &head = message->getHeader();
	auto callID = head["Call-ID"];

	switch(message->getType()) {
	case sipRegister:
		doRegister(message);
		break;
	default:
		if(message->getMethod().length()) {
			printf("call router forwardMessage\r\n");
			m_router->forwardMessage(shared_from_this(), message);
			printf("call router forwardMessage done\r\n");
		} else {
			printf("call router forwardStatus\r\n");
			m_router->forwardStatus(shared_from_this(), message);
			printf("call router forwardStatus done\r\n");
		}
		break;
	}
}

void SIPEndpoint::doRegister(const std::shared_ptr<SIPMessage>& message)
{
	std::ostringstream os;
	static std::once_flag init_regex_flag;
	static regex_t m_fromRegex;

	std::call_once(init_regex_flag, [](){
		int status;
		char errbuf[2048];

		const char *patten = "<sip:(.+@.+)>";
		status = regcomp(&m_fromRegex, patten, REG_EXTENDED);
		if(status < 0) {
			regerror(status, &m_fromRegex, errbuf, sizeof(errbuf));
		}
	});

	auto &head = message->getHeader();
	std::string from = head["From"];

	const char *fromvalue = from.c_str();
	regmatch_t match[20];
	auto err = regexec(&m_fromRegex, fromvalue, 20, match, 0);
	if(err == 0) {
		std::string UA(&fromvalue[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
		m_ua = UA;
	} else {
		return;
	}

	auto resp = message->makeResponse(200, "OK", "", "");

	auto THIS = shared_from_this();
	m_router->registerEndpoint(THIS);

	this->sendMessage(resp);
}