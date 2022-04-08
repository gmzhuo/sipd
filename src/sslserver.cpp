#include "sslserver.h"

int sslEndpoint::sendMessage(const std::shared_ptr<SIPMessage>& message)
{
	auto msg = message->toString();

	m_sslSession->sendMessage(msg.c_str(), msg.length());
	printf("send msg %s\r\n", msg.c_str());
	return 0;
}
