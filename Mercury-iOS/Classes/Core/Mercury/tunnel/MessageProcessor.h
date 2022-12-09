#pragma once
#include "../socket/MsgProtocol.h"
#include "TunnelDefine.h"

namespace mercury {

class MessageProcessor
{
public:
    MessageProcessor(const MessageProtocol& msg);
    ~MessageProcessor();
    void Process();
    uint8_t GetMessageCommand() const;
    uint64_t GetMessageId() const;
    std::vector<std::string> GetAckMessageIds()const;
    MessageType GetMessageType() const;
    uint8_t GetMessageTwoWay() const;
    std::vector<ExtensionHeader_t> GetMessageHeaders() const;
    std::vector<uint8_t> GetMessageBodyData() const;
private:
    void process_header(const std::vector<ExtensionHeader_t>& headers);
    void process_body(const std::vector<uint8_t>& msgData);
    std::vector<uint8_t> uncompressed_body(const std::vector<uint8_t>& body);
private:
    std::string messageId_;
    std::vector<std::string> ackMessageId_;
    std::vector<uint8_t> msgBodyData_;
    MessageProtocol msg_;
    std::vector<ExtensionHeader_t> buisinessMessageHeaders_;
    const uint32_t RESERVE_UNCPMPRESS_DATA_SIZE = 10*1024;
    const uint32_t MAX_UNCPMPRESS_DATA_SIZE = 50* 1024 * 1024;
    CompressType compressType_ = CompressType::no;
    std::vector<uint8_t> uncompressedBuffer_;
};
}

