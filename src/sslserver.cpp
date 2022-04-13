#include "sslserver.h"

int sslEndpoint::sendMessage(const std::shared_ptr<SIPMessage>& message)
{
	auto &msg = message->getMessage();
	m_sslSession->sendMessage(msg.c_str(), msg.length());
	return 0;
}
