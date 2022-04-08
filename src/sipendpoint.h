#pragma once
#include "siprouter.h"

class SIPMessage;

class SIPEndpoint: public std::enable_shared_from_this<SIPEndpoint> {
public:
	SIPEndpoint(SIPRouter *router);
	virtual ~SIPEndpoint();
public:
	virtual int sendMessage(const std::shared_ptr<SIPMessage>& message) = 0;
public:
	void onSessionClosed();
	void onMessage(const std::shared_ptr<SIPMessage>& message);
protected:
	void doRegister(const std::shared_ptr<SIPMessage>& message);
private:
  SIPRouter *m_router;
  std::shared_ptr<SIPEndpoint> m_myself;
};
