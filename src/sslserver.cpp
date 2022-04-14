#include "sslserver.h"

int sslEndpoint::sendMessage(const std::shared_ptr<SIPMessage>& message)
{
	auto &msg = message->getMessage();
	printf("ep %s send:\r\n%s", m_ua.c_str(), msg.c_str());
	m_sslSession->sendMessage(msg.c_str(), msg.length());
	return 0;
}
