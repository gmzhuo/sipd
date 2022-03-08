#include "sipmessage.h"
#include <sstream>

SIPMessage::SIPMessage()
{
}

SIPMessage::SIPMessage(std::string destCommunication, std::string destTarget, bool isSolicit):
  m_destCommunication(destCommunication), m_destTarget(destTarget), m_isSolicit(isSolicit)
{
}

SIPMessage::~SIPMessage()
{
}

std::string SIPMessage::toString() const
{
  ostringstream os;

  os << m_method << " " << m_ruri << SIP/2.0\r\n";
  for(auto it = m_headers.begin(); it != m_headers.end(); ++it) {
    os << it->first << ": " << it->second << "\r\n";
  }

  for(auto it = m_extraHeaders.begin(); it != m_extraHeaders.end(); ++it) {
    os << *it << "\r\n";
  }

  if(m_body.length()) {
    os << "Content-Length: " <<m_body.length() << "\r\n\r\n";
    os << m_body;
  } else {
    os << "Content-Length: 0\r\n\r\n";
  } 
}

bool SIPMessage::isSolicit() const
{
  return m_isSolicit;
}

const std::string& SIPMessage::getDestCommunication() const
{
  return m_destCommunication;
}

const std::string& SIPMessage::getDestTarget() const
{
  return m_destTarget;
}
