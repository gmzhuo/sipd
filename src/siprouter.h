#pragma once
#include <string>
#include <memory>
#include <map>

class SIPEndpoint;
class SIPMessage;

class SIPRouter{
public:
  SIPRouter(const std::string realm);
  virtual ~SIPRouter() = default;
  SIPRouter(const SIPRouter& router) = delete;
  SIPRouter& operator=(const SIPRouter& router) = delete;
protected:
  void forwardOutput(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message);
public:
  void onSessionClosed(std::shared_ptr<SIPEndpoint>& ep);
public:
  void forwardMessage(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message);
  void forwardSolicit(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message);
  void registerEndpoint(std::shared_ptr<SIPEndpoint>& ep, const std::string& ua, const std::string& communication);
private:
  std::map<std::string, std::map<std::string, std::shared_ptr<SIPEndpoint>>> m_endpoints;
  std::map<std::string, std::map<std::string, std::shared_ptr<SIPEndpoint>>> m_endpointsMapByCommunication;
  std::string m_realm;
};
