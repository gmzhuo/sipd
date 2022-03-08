#pragma once

class SIPMessage{
public:
  SIPMessage();
  virtual ~SIPMessage();
public:
  virtual bool isSolicit() const;
};
