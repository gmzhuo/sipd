#include "siprouter.h"
#include "sipendpoint.h"
#include "sipmessage.h"

SIPRouter::SIPRouter(const std::string realm):
  m_realm(realm)
{
}

void SIPRouter::forwardMessage(const SIPEndpoint& from, const SIPMessage& message)
{
  std::string realm = message.getRealm();
  if(realm != m_realm) {
    this->forwardOutput(from, message);
    return;
  }

  std::string target = message.getDestCommunication();
  std::string ua = message.getDestTarget();
  auto it = m_endpoints.find(ua);

  if(it != m_endpointsMapByCommunication.end()) {
    auto eit = it->second.find(target);
    if(eit != it->second.end()) {
      eit->second->sendMessage(message);
    }
  }
}


void SIPRouter::forwardSolicit(const SIPEndpoint& from, const SIPMessage& message)
{
  std::string realm = message.getRealm();
  if(realm != m_realm) {
    this->forwardOutput(from, message);
    return;
  }

  std::string ua = message.getDestTarget();
  auto it = m_endpoints.find(ua);
  if(it != m_endpoints.end()) {
    for(auto eip = it->second.begin(); eip != it->second.end(); ++eip) {
      eip->second->sendMessage(message);
    }
  }
}

void SIPRouter::forwardOutput(const SIPEndpoint& from, const SIPMessage& message)
{
  //create or find the connection to remote server;
  std::shared_ptr<SIPEndpoint> remoteServer;

  if(remoteServer) {
    remoteServer->sendMessage(message);
  }
}

void SIPRouter::registerEndpoint(std::shared_ptr<SIPEndpoint> &ep, const std::string& ua, const std::string& communication)
{
  auto it = m_endpoints.find(ua);
  if(it != m_endpoints.end()) {
    it->second[communication] = ep;
  } else {
    std::map<std::string, std::shared_ptr<SIPEndpoint>> eps;
    eps[communication] = ep;
    m_endpoints[ua] = eps;
  }
}
