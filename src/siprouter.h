#pragma once
class SIPEndpoint;
class SIPMessage;

class SIPRouter{
protected:
  SIPRouter() = default;
  virtual ~SIPRouter() = default;
  SIPRouter(const SIPRouter& router) = deleted;
  SIPRouter& operator=(const SIPRouter& router) = deleted;
public:
  void forwardMessage(const SIPEndpoint& from, const SIPMessage& message);
  void forwardSolicit(const SIPEndpoint& from, const SIPMessage& message);
  void registerEndpoint(std::shared_ptr<SIPEndpoint>, const std::string& ua, const std::string& communication);
private:
  std::map<std::string, std::shared_ptr<SIPEndpoint>> m_endpointsMapByCommunication;
  std::map<std::string, std::list<std::shared_ptr<SIPEndpoint>>> m_endpointsMapByName;
};
