#include "sipmessage.h"

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
