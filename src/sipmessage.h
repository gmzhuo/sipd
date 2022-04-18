#pragma once
#include <regex.h>
#include <string>
#include <map>
#include <list>
#include <memory>

typedef enum SIPMessageType{
	sipRegister,
	sipInvite,
	sipACK,
	sipBye,
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
private:
	std::string toString() const;
public:
	std::shared_ptr<SIPMessage> makeResponse(unsigned short status, const char *reason,
		const char *extension, const char *content,
		std::map<std::string, std::string> extraHeades=std::map<std::string, std::string>()) const;
	SIPMessageType getType() const {
		return m_type;
	}
	const char *getTypeString() const;
	const std::string& getMethod() const
	{
		return m_method;
	}
	const std::string& getTarget() const
	{
		return m_target;
	}
	const SIPMessageHead& getHeader() const
	{
		return m_headers;
	}
	const std::string& getMessage() const
	{
		return m_message;
	}
public:
	std::string m_message;
	unsigned short m_status;
	std::string m_target;
	std::string m_method;
	std::string m_reason;
	std::string m_version;
	std::string m_content;
	
	SIPMessageHead m_headers;
	
	SIPMessageType m_type = sipInvalid;
};
