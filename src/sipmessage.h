#pragma once

class SIPMessage{
public:
  SIPMessage();
  virtual ~SIPMessage();
public:
  virtual bool isSolicit() const;
  const std::string& getDestCommunication() const;
  const std::string& getDestTarget() const;
private:
  std::string m_destCommunication;
  std::string m_destTarget;
};
