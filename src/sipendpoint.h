#pragma once
#include "siprouter.h"

class SIPMessage;

class SIPEndpoint {
public:
  SIPEndpoint(SIPRouter *router);
  virtual ~SIPEndpoint();
public:
  virtual int sendMessage(const SIPMessage& message);
protected:
  virtual void onMessage(const SIPMessage& message);
private:
  SIPRouter *m_router;
};
