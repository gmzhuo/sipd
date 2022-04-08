#pragma once
#include <regex.h>
#include <string>
#include <map>
#include <list>
#include <memory>

typedef enum SIPMessageType{
	sipRegister,
	sipInvite,

	sipInvalid
} SIPMessageType;


class SIPMessageHead{
public:
	SIPMessageHead() = default;
	virtual ~SIPMessageHead() = default;
public:
	std::string operator[](const char *) const;
	std::string& operator[](const char *);
public:
	std::map<std::string, std::string> m_headers;
};

class SIPMessage{
public:
  SIPMessage();
  SIPMessage(const char *data, size_t length);
  virtual ~SIPMessage();
public:
  std::shared_ptr<SIPMessage> makeResponse(unsigned short status, const char *reason,
		const char *extension, const char *content) const;
  std::string toString() const;
  SIPMessageType getType() const {
	  return m_type;
  }
  const char *getTypeString() const;
public:
  unsigned short m_status;
  std::string m_method;
  std::string m_reason;
  std::string m_version;
  std::string m_content;
 
  SIPMessageHead m_headers;
 
  SIPMessageType m_type = sipInvalid;
};
