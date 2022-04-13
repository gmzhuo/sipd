#pragma once
#include <string>
#include <memory>
#include <map>
#include <list>

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
  void forwardStatus(const std::shared_ptr<SIPEndpoint>& from, const std::shared_ptr<SIPMessage>& message);
  void registerEndpoint(std::shared_ptr<SIPEndpoint>& ep);
private:
  std::map<std::string, std::map<void *, std::shared_ptr<SIPEndpoint>>> m_endpoints;
  std::map<std::string, std::map<std::string, std::weak_ptr<SIPEndpoint>>> m_endpointsMapByCallID;
  std::string m_realm;
};
