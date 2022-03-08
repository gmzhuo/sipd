#include "siprouter.h"

void SIPRouter::forwardMessage(const SIPEndpoint& from, const SIPMessage& message)
{
  std::string target = message.getDestCommunication();
  auto it = m_endpointsMapByCommunication.find(target);
  if(it != m_endpointsMapByCommunication.end()) {
    it->second->sendMessage(message);
  }
}


void SIPRouter::forwardSolicit(const SIPEndpoint& from, const SIPMessage& message)
{
  std::string target = message.getDestTarget();
  auto it = m_endpointsMapByName.find(target);
  if(it != m_endpointsMapByName.end()) {
    for(auto eip = it->second->begin(); eip != it->second->end; ++eip) {
      (*eip)->sendMessage(message);
    }
  }
}
