#pragma once
#include <string>
#include <map>
#include <list>

class SIPMessage{
public:
  SIPMessage();
  SIPMessage(std::string destCommunication, std::string destTarget, bool isSolicit);
  virtual ~SIPMessage();
public:
  virtual bool isSolicit() const;
  const std::string& getDestCommunication() const;
  const std::string& getDestTarget() const;
  std::string toString() const;
  std::string getRealm() const;
private:
  std::string m_method;
  std::string m_ua;
  std::string m_ruri;
  std::string m_body;
  std::map<std::string, std::string> m_headers;
  std::list<std::string> m_extraHeaders;
  std::string m_destCommunication;
  std::string m_destTarget;
  bool m_isSolicit;
};
