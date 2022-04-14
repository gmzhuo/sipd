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
	printf("endpoint destructed\r\n");
}

void SIPEndpoint::onSessionClosed()
{
	printf("in end point on closed %p\r\n", this);

	printf("in end point on closed call router\r\n");
	auto THIS = shared_from_this();
	m_router->onSessionClosed(THIS);
	printf("in end point on closed call router done %ld\r\n", THIS.use_count());
}

void SIPEndpoint::onMessage(const std::shared_ptr<SIPMessage>& message)
{
	auto type = message->getTypeString();
	std::cout << "on message " << type << std::endl;

	auto &head = message->getHeader();
	auto callID = head["Call-ID"];

	std::cout << "Call-ID: " << callID << " length:" << callID.length() <<std::endl;

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
			m_router->forwardStatus(shared_from_this(), message);
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
			printf("compile %s %d %s\r\n", patten, status, errbuf);
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
		std::cout << "finded UA: " << m_ua << " " << match[0].rm_so << match[0].rm_eo <<std::endl;
	} else {
		std::cout << "failed to find UA from " << fromvalue << std::endl;
		return;
	}

	auto resp = message->makeResponse(200, "OK", "", "");

	auto THIS = shared_from_this();
	m_router->registerEndpoint(THIS);

	std::cout << "recived reg request:" << std::endl << message->getMessage() << std::endl;
	std::cout << "send reg response:" << std::endl << resp->getMessage() << std::endl;
	this->sendMessage(resp);
}