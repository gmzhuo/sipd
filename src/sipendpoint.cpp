#include <iostream>
#include <sstream>
#include <mutex>
#define OPENSSL_NO_DEPRECATED_3_0
#include <openssl/md5.h>
#include "sipendpoint.h"
#include "sipmessage.h"
#include "sqlite3.h"
#include "callid.h"

SIPEndpoint::SIPEndpoint(boost::asio::io_context &context, SIPRouter *router):
	m_router(router), m_context(context)
{
	m_pendingMessage = std::make_shared<SIPMessage>();
}

SIPEndpoint::~SIPEndpoint()
{
}

void SIPEndpoint::onEndpointClosed()
{
	auto THIS = shared_from_this();
	m_router->onEndpointClosed(THIS);
}

void SIPEndpoint::onBuffer(const char *buf, size_t length)
{
	char line[1024];
	char content[8192];

	static std::once_flag init_regex_flag;
	static regex_t m_statusRegex;
	static regex_t m_cmdlineRegex;
	static regex_t m_headlineRegex;
	static regex_t m_fromRegex;
	std::call_once(init_regex_flag, [](){
		int status;
		char errbuf[2048];

		const char *patten = "SIP/(.*) (d+) (.*)";
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

	std::string value(buf, length);
	std::istringstream is(value);

	while(!is.eof()) {
		switch(m_parseState) {
		case messageParseStateFirstLine:
			is.getline(line, sizeof(line));
			if(strlen(line) <= 1) {
				continue;
			}

			m_parseState = messageParseStateHeads;

			do {
				regmatch_t match[20];
				auto err = regexec(&m_cmdlineRegex, line, 20, match, 0);
				if(err == 0) {
					std::string method(&line[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
					std::string version(&line[match[3].rm_so], match[3].rm_eo - match[3].rm_so);
					std::string target(&line[match[2].rm_so], match[2].rm_eo - match[2].rm_so);

					printf("method %s version %s target %s\r\n", method.c_str(), version.c_str(), target.c_str());
					m_pendingMessage->setMethod(method, version, target);
					break;
				}

				err = regexec(&m_statusRegex, line, 20, match, 0);
				if(err == 0) {
					std::string version(&line[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
					std::string reason(&line[match[3].rm_so], match[3].rm_eo - match[3].rm_so);
					std::string status(&line[match[2].rm_so], match[2].rm_eo - match[2].rm_so);

					printf("version %s reason %s status %s\r\n", version.c_str(), reason.c_str(), status.c_str());
					m_pendingMessage->setStatus(version, status, reason);
				}
			}while(false);

			break;
		case messageParseStateHeads:
			is.getline(line, sizeof(line));
			if(strlen(line) <= 1) {
				m_pendingMessage->addContent(buf, 0);
				this->onMessage(m_pendingMessage);
				m_pendingMessage = std::make_shared<SIPMessage>();
				m_parseState = messageParseStateFirstLine;
			} else {
				regmatch_t match[20];
				auto err = regexec(&m_headlineRegex, line, 20, match, 0);
				if(err == 0) {
					std::string head(&line[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
					std::string value(&line[match[2].rm_so], match[2].rm_eo - match[2].rm_so - 1);
					if(head == "Content-Length") {
						m_parseState = messageParseStateContent;
						m_contentLength = atoi(value.c_str());
						if(m_contentLength == 0) {
							is.getline(line, sizeof(line));
							m_pendingMessage->addContent(buf, 0);
							this->onMessage(m_pendingMessage);
							m_pendingMessage = std::make_shared<SIPMessage>();
							m_parseState = messageParseStateFirstLine;
						}
						continue;
					}
				}
				m_pendingMessage->addLine(line);
			}
			break;
		case messageParseStateContent:
			length = is.readsome(content, (m_contentLength >= sizeof(content))?(sizeof(content) - 1):m_contentLength);
			content[length] = 0;
			if(length > 0) {
				content[length] = 0;
				m_pendingMessage->addContent(content, length);
				m_contentLength -= length;
				if(m_contentLength == 0) {
					m_pendingMessage->addContent(content, 0);
					this->onMessage(m_pendingMessage);
					m_pendingMessage = std::make_shared<SIPMessage>();
					m_parseState = messageParseStateFirstLine;
					is.getline(line, sizeof(line));
				}
			}
			break;
		}
	};
}

void SIPEndpoint::onMessage(const std::shared_ptr<SIPMessage>& message)
{
	auto type = message->getTypeString();

	printf("ep %s recive:\r\n\t%s", m_ua.c_str(), message->getMessage().c_str());
	auto &head = message->getHeader();
	auto callID = head["Call-ID"];

	switch(message->getType()) {
	case sipRegister:
		doRegister(message);
		break;
	case sipBye:
	case sipACK:
	case sipInvalid:
		m_router->forwardStatus(shared_from_this(), message);
		break;
	default:
		m_router->forwardMessage(shared_from_this(), message);
		break;
	}
}

void SIPEndpoint::doRegister(const std::shared_ptr<SIPMessage>& message)
{
	std::ostringstream os;
	static std::once_flag init_regex_flag;
	static regex_t m_fromRegex;

	std::call_once(init_regex_flag, [](){
		int status;
		char errbuf[2048];

		const char *patten = "<sip:(.+@.+)>";
		status = regcomp(&m_fromRegex, patten, REG_EXTENDED);
		if(status < 0) {
			regerror(status, &m_fromRegex, errbuf, sizeof(errbuf));
		}
	});

	auto &head = message->getHeader();
	std::string from = head["From"];

	const char *fromvalue = from.c_str();
	regmatch_t match[20];
	auto err = regexec(&m_fromRegex, fromvalue, 20, match, 0);
	if(err == 0) {
		std::string UA(&fromvalue[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
		m_ua = UA;
	} else {
		return;
	}

	auto makeAuthorizeResponse = [this, message]() {
		std::map<std::string, std::string> extraHeades;
		char auvalue[512];
		m_nonce = "";
		for(int j = 0; j < 3; ++j) {
			int r = random();
			char value[32];
			sprintf(value, "%08x", r);
			m_nonce += value;
		}
		sprintf(auvalue, "Digest realm=\"%s\", nonce=\"%s\", algorithm=MD5", m_router->getRealm().c_str(), m_nonce.c_str());
		extraHeades["WWW-Authenticate"] =  auvalue;
		auto resp = message->makeResponse(401, "Unauthorized", "", "", 1, extraHeades);
		return resp;
	};

	auto makeAuthorizedResponse = [this, message]() {
		auto resp = message->makeResponse(200, "OK", "", "", 600);
		return resp;
	};

	std::shared_ptr<SIPMessage> resp;
	if(checkAuthorize(message)) {
		resp = makeAuthorizedResponse();
		//store device info here.
		std::string devType, devID;
		sqlite3DB::getInstance().addDeviceInfo(m_ua.c_str(), devType.c_str(), devID.c_str());

		auto THIS = shared_from_this();
		m_router->registerEndpoint(THIS);
	} else {
		resp = makeAuthorizeResponse();
	}

	this->forwardMessage(resp);
}

bool SIPEndpoint::checkAuthorize(const std::shared_ptr<SIPMessage>& message)
{
	static std::once_flag init_regex_flag;
	static regex_t m_responseRegex;
	auto pos = m_ua.find('@');
	auto username = m_ua.substr(0, pos);

	std::call_once(init_regex_flag, [](){
		int status;
		char errbuf[2048];

		const char *patten = "response=\"(.+)\"";
		status = regcomp(&m_responseRegex, patten, REG_EXTENDED);
		if(status < 0) {
			regerror(status, &m_responseRegex, errbuf, sizeof(errbuf));
		}
	});

	auto &head = message->getHeader();
	std::string authorization = head["Authorization"];
	const char *authinfo = authorization.c_str();

	unsigned char ha1[16], ha2[16], response[16];
	char ha1str[64],ha2str[64],responsestr[64];

	if(authorization.length() == 0)
		return false;

	std::string m_realm = m_router->getRealm();
	std::string m_password = sqlite3DB::getInstance().getUserPassword(username.c_str());

	std::string a1 = username + ":" + m_realm + ":" + m_password;
	MD5((const unsigned char*)a1.c_str(), a1.length(), ha1);
	sprintf(ha1str, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			ha1[0], ha1[1], ha1[2], ha1[3], 
			ha1[4], ha1[5], ha1[6], ha1[7], 
			ha1[8], ha1[9], ha1[10], ha1[11], 
			ha1[12], ha1[13], ha1[14], ha1[15]);

	std::string a2 = "REGISTER:sip:www.ipv4.mtlink.cn";
	MD5((const unsigned char*)a2.c_str(), a2.length(), ha2);
	sprintf(ha2str, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			ha2[0], ha2[1], ha2[2], ha2[3], 
			ha2[4], ha2[5], ha2[6], ha2[7], 
			ha2[8], ha2[9], ha2[10], ha2[11], 
			ha2[12], ha2[13], ha2[14], ha2[15]);

	std::string finalstr = std::string(ha1str) + ":" + m_nonce + ":" + std::string(ha2str);
	MD5((const unsigned char*)finalstr.c_str(), finalstr.length(), response);

	sprintf(responsestr, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			response[0], response[1], response[2], response[3], 
			response[4], response[5], response[6], response[7], 
			response[8], response[9], response[10], response[11], 
			response[12], response[13], response[14], response[15]);

	regmatch_t match[20];
	auto err = regexec(&m_responseRegex, authinfo, 20, match, 0);
	if(err == 0) {
		std::string responseValue(&authinfo[match[1].rm_so], match[1].rm_eo - match[1].rm_so);
		if(strncasecmp(responseValue.c_str(), responsestr, 32) == 0) {
			return true;
		}
	}

	return false;
}

void SIPEndpoint::removeCallID(const std::string& id)
{
	printf("remove call ID %s ua %s\r\n", id.c_str(), m_ua.c_str());
	auto it = m_callIDs.find(id);

	if(it != m_callIDs.end()) {
		m_callIDs.erase(it);
	}
}

bool SIPEndpoint::haveCallID(const std::string& id) const
{
	auto it = m_callIDs.find(id);

	if(it != m_callIDs.end()) {
		return true;
	}

	return false;
}

void SIPEndpoint::addCallID(const std::string& id)
{
	printf("add call ID %s ua %s\r\n", id.c_str(), m_ua.c_str());
	auto it = m_callIDs.find(id);
	if(it == m_callIDs.end()) {
		m_callIDs[id] = std::make_shared<callID>(m_context, id.c_str(), this);
	}
}

int SIPEndpoint::forwardMessage(const std::shared_ptr<SIPMessage>& message)
{
	printf("ep %s send:\r\n\t%s", m_ua.c_str(), message->getMessage().c_str());
	return sendMessage(message);
}

