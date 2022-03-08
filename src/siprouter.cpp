#include "siprouter.h"

void SIPRouter::forwardMessage(const SIPEndpoint& from, const SIPMessage& message)
{
  std::string target = message.getDestCommunication();
  std::string ua = message.getDestTarget();
  auto it = m_endpoints.find(ua);

  if(it != m_endpointsMapByCommunication.end()) {
    auto eit = it->second->find(target);
    if(eit != it->second->end()) {
      eit->second->sendMessage(message);
    }
  }
}


void SIPRouter::forwardSolicit(const SIPEndpoint& from, const SIPMessage& message)
{
  std::string ua = message.getDestTarget();
  auto it = m_endpoints.find(ua);
  if(it != m_endpoints.end()) {
    for(auto eip = it->second->begin(); eip != it->second->end(); ++eip) {
      eip->second->sendMessage(message);
    }
  }
}

void SIPRouter::registerEndpoint(std::shared_ptr<SIPEndpoint> &ep, const std::string& ua, const std::string& communication)
{
  auto it = m_endpoints.find(ua);
  if(it != m_endpoints.end()) {
    it->second[communication] = ep;
  } else {
    std::map<std::string std::shared_ptr<SIPEndpoint>> eps;
    eps[communication] = ep;
    m_endpoints[ua] = eps;
  }
}
