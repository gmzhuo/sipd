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

SIPMessage::SIPMessage(const char *data, size_t length)
{
	m_message = std::string(data, length);
	static std::once_flag init_regex_flag;
	static regex_t m_statusRegex;
	static regex_t m_cmdlineRegex;
	static regex_t m_headlineRegex;
	static regex_t m_fromRegex;
	std::call_once(init_regex_flag, [](){
		int status;
		char errbuf[2048];

		const char *patten = "SIP/(d+.d+) (d+) (.*)";
		status = regcomp(&m_statusRegex, patten, REG_EXTENDED);
		if(status < 0) {
			regerror(status, &m_statusRegex, errbuf, sizeof(errbuf));
			printf("compile %s %d %s\r\n", patten, status, errbuf);
		}

		patten = "(.+) (.+) SIP/(.+)";
		status = regcomp(&m_cmdlineRegex, patten, REG_EXTENDED);
		if(status < 0) {
			regerror(status, &m_cmdlineRegex, errbuf, sizeof(errbuf));
			printf("compile %s %d %s\r\n", patten, status, errbuf);
		}

		patten = "(.+): (.*)";
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

	m_type = sipInvalid;

	std::string value(data, length);
	std::istringstream is(value);

	try {
		char line[1024];
		is.getline(line, sizeof(line));
		bool isStatus = false;

		do {
			regmatch_t match[20];
			auto err = regexec(&m_cmdlineRegex, line, 20, match, 0);
			if(err == 0) {
				std::string method(&line[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
				m_method = method;
				std::string version(&line[match[3].rm_so], match[3].rm_eo - match[3].rm_so);
				m_version = version;
				std::string target(&line[match[2].rm_so], match[2].rm_eo - match[2].rm_so);
				m_target = target;
				m_type = getMessageType(method.c_str());
				break;
			}

			isStatus = true;
			err = regexec(&m_statusRegex, line, 20, match, 0);
			if(err == 0) {
				printf("status message %s\r\n", data);
			}
			
		}while(false);

		while(!is.eof()) {
			is.getline(line, sizeof(line));

			size_t len = strlen(line);
			if(len) {
				regmatch_t match[20];
				auto err = regexec(&m_headlineRegex, line, 20, match, 0);
				if(err == 0) {
					std::string head(&line[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
					std::string value(&line[match[2].rm_so], match[2].rm_eo - match[2].rm_so - 1);
					m_headers[head.c_str()] = value.c_str();
					if(isStatus) {
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
		printf("message target %s\r\n", m_target.c_str());
	} catch(...) {
	}
}

SIPMessage::~SIPMessage()
{
}

std::shared_ptr<SIPMessage> SIPMessage::makeResponse(
	unsigned short status, const char *reason, const char *extension, const char *content) const
{
	auto result = std::make_shared<SIPMessage>();
	result->m_status = status;
	result->m_reason = reason;
	result->m_version = this->m_version;

	result->m_headers["Via"] = this->m_headers["Via"];
	result->m_headers["To"] = this->m_headers["To"];
	result->m_headers["From"] = this->m_headers["From"];
	result->m_headers["Call-ID"] = this->m_headers["Call-ID"];
	result->m_headers["CSeq"] = this->m_headers["CSeq"];
	result->m_headers["Contact"] = this->m_headers["Contact"];

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
		os << m_method << " " << m_target << " SIP/" << m_version << " \r\n";
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


