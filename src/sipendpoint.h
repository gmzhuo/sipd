#pragma once
#include "siprouter.h"
#include <boost/asio/ip/tcp.hpp>

class SIPMessage;

typedef enum sipMessageParseState{
	messageParseStateFirstLine,
	messageParseStateHeads,
	messageParseStateContent
} sipMessageParseState;

class SIPEndpoint: public std::enable_shared_from_this<SIPEndpoint> {
public:
	SIPEndpoint(boost::asio::io_context &context, SIPRouter *router);
	virtual ~SIPEndpoint();
public:
	virtual int sendMessage(const std::shared_ptr<SIPMessage>& message) = 0;
public:
	int forwardMessage(const std::shared_ptr<SIPMessage>& message);
	bool haveCallID(const std::string& id) const;
	void removeCallID(const std::string& id);
	void addCallID(const std::string& id);
	void onEndpointClosed();
	void onBuffer(const char *buf, size_t length);
	void onMessage(const std::shared_ptr<SIPMessage>& message);
protected:
	bool checkAuthorize(const std::shared_ptr<SIPMessage>& message);
	void doRegister(const std::shared_ptr<SIPMessage>& message);
public:
	const std::string& getUA() const
	{
		return m_ua;
	}
protected:
	SIPRouter *m_router;
	std::string m_ua;
	std::string m_nonce;
	std::map<std::string, std::shared_ptr<callID>> m_callIDs;
	boost::asio::io_context& m_context;
	sipMessageParseState m_parseState = messageParseStateFirstLine;
	std::shared_ptr<SIPMessage> m_pendingMessage;
	size_t m_contentLength;
};
