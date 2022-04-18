#include "sipmessage.h"
#include <sstream>
#include <iostream>
#include <cstring>
#include <streambuf>
#include <mutex>
#include <sys/types.h>

const char *SIPMessageStringType[sipInvalid + 2] = {
	"REGISTER",
	"INVITE",
	"ACK",
	"BYE",
	"INVALID",
	NULL
};

static struct statusCode{
	unsigned short code;
	const char *status;
} statusCodeList[]= {
	{401, "Unauthorized"},
	{200, "Success"},
	{180, "Trying"},
};

const char *getStatusCode(unsigned short status)
{
	static auto listSize = sizeof(statusCodeList)/sizeof(statusCodeList[0]);
	for(int j = 0; j < listSize; ++j) {
		if(status == statusCodeList[j].code)
			return statusCodeList[j].status;
	}

	return "unknow";
}

static SIPMessageType getMessageType(const char *message)
{
	for(int i = 0;; ++i) {
		if(!SIPMessageStringType[i])
			return sipInvalid;
		if(strncmp(SIPMessageStringType[i], message, strlen(SIPMessageStringType[i])) == 0) {
			return (SIPMessageType)i;
		}
	}

	return sipInvalid;
}

std::string SIPMessageHead::operator[](const char *attr) const
{
	std::string result;

	auto it = m_headers.find(attr);
	if(it != m_headers.end()) {
		return it->second;
	}

	return result;
}

std::string& SIPMessageHead::operator[](const char *attr)
{
	auto it = m_headers.find(attr);
	if(it == m_headers.end()) {
		m_headers[attr] = "";
	}	

	return m_headers[attr];
}

SIPMessage::SIPMessage()
{
}

void SIPMessage::setMethod(const std::string& method, const std::string& version,
			const std::string& target)
{
	m_method = method;
	m_version = version;
	m_target = target;

	m_type = getMessageType(method.c_str());
}

void SIPMessage::setStatus(const std::string& version, const std::string& status,
			const std::string& reason)
{
	m_status = atoi(status.c_str());
	m_reason = reason;
	m_version = version;
}

void SIPMessage::addLine(const char *line)
{
	static std::once_flag init_regex_flag;
	static regex_t m_headlineRegex;
	static regex_t m_fromRegex;

	std::call_once(init_regex_flag, [](){
		int status;
		char errbuf[2048];

		const char *patten = "(.+): (.*)";
		status = regcomp(&m_headlineRegex, patten, REG_EXTENDED);
		if(status < 0) {
			regerror(status, &m_headlineRegex, errbuf, sizeof(errbuf));
			printf("compile %s %d %s\r\n", patten, status, errbuf);
		}

		patten = "<sip:(.+@.+)>";
		status = regcomp(&m_fromRegex, patten, REG_EXTENDED);
		if(status < 0) {
			regerror(status, &m_fromRegex, errbuf, sizeof(errbuf));
		}
	});

	size_t len = strlen(line);
	if(len) {
		regmatch_t match[20];
		auto err = regexec(&m_headlineRegex, line, 20, match, 0);
		if(err == 0) {
			std::string head(&line[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
			std::string value(&line[match[2].rm_so], match[2].rm_eo - match[2].rm_so - 1);
			m_headers[head.c_str()] = value.c_str();
			if(m_method.length() == 0) {
				if(head == "From") {
					auto err = regexec(&m_fromRegex, line, 20, match, 0);
					if(err == 0) {
						std::string UA(&line[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
						m_target = UA;
					}
				}
			} else {
				if(head == "To") {
					auto err = regexec(&m_fromRegex, line, 20, match, 0);
					if(err == 0) {
						std::string UA(&line[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
						m_target = UA;
					}
				} 
			}
		} else {
		}
	}
}

void SIPMessage::addContent(const char *content, size_t length)
{
	if(length == 0) {
		m_message = this->toString();
	} else {
		m_content += content;
	}
}

SIPMessage::~SIPMessage()
{
}

std::shared_ptr<SIPMessage> SIPMessage::makeResponse(
	unsigned short status, const char *reason, const char *extension, const char *content,
	unsigned int timeout, std::map<std::string, std::string> extraHeades) const
{
	char expires[64];
	sprintf(expires, "expires=%d", timeout);
	auto result = std::make_shared<SIPMessage>();
	result->m_status = status;
	result->m_reason = reason;
	result->m_version = this->m_version;

	result->m_headers["Via"] = this->m_headers["Via"];
	result->m_headers["To"] = this->m_headers["To"];
	result->m_headers["From"] = this->m_headers["From"];
	result->m_headers["Call-ID"] = this->m_headers["Call-ID"];
	result->m_headers["CSeq"] = this->m_headers["CSeq"];
	result->m_headers["Contact"] = this->m_headers["Contact"] + ";" + expires;

	for(auto it = extraHeades.begin(); it != extraHeades.end(); ++it) {
		result->m_headers[it->first.c_str()] = it->second;
	}

	if(content) {
		result->m_content = m_content;
	}

	result->m_message = result->toString();

	return result;
}

const char *SIPMessage::getTypeString() const
{
	if(m_type <= sipInvalid) {
		return SIPMessageStringType[m_type];
	}

	return SIPMessageStringType[sipInvalid];
}

std::string SIPMessage::toString() const
{
	std::ostringstream os;

	//printf("SIP %s %d %s\r\n",  m_status, m_reason.c_str());
	if(m_method.length()) {
		os << m_method << " sip:" << m_target << " SIP/" << m_version << "\n";
	} else {
		os << "SIP/" << "2.0" << " " << m_status << " " << m_reason << "\r\n";
	}

	auto headers = m_headers.m_headers;
	for(auto it = headers.begin(); it != headers.end(); ++it) {
		os << it->first << ": " << it->second << "\r\n";
	}

	os << "Content-Length: " << m_content.length() << "\r\n";
	os << m_content;
	os << "\r\n";

	return os.str();
}


