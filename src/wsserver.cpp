#include "wsserver.h"

int wsEndpoint::sendMessage(const std::shared_ptr<SIPMessage>& message)
{
	auto &msg = message->getMessage();

	m_wsSession->sendMessage(msg.c_str(), msg.length());

	return msg.length();
}

