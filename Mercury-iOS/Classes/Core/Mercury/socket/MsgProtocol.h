#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Constants.h"

namespace mercury {

const int PROTO_VERSION = 4;

#pragma pack(1)
struct Meta_Fixed_t
{
    MessageType type = MessageType::REQ;
    uint8_t twoWay = 1;
    uint8_t heartbeat = 0;
    uint8_t version = 0;
};

#pragma pack(1)
struct Meta_t
{
    Meta_Fixed_t fixedInfo;
    uint64_t msgId;
    int8_t command;
    uint64_t payloadLength;
};

#pragma pack()
struct ExtensionHeader_t
{
    uint8_t     headerType;
    std::vector<uint8_t> headerData;
};

class MessageProtocol
{
public:
    
    MessageProtocol();
    ~MessageProtocol();
    MessageProtocol(const MessageProtocol& right);
    MessageProtocol& operator=(const MessageProtocol& right)= delete;
    void SetMeta(const Meta_t& meta);
    void AddExtensionHeader(uint8_t type, const std::vector<uint8_t>& headerData);
    void SetBodyData(const std::vector<uint8_t>& bodyData);
    void SetPayload(const std::vector<uint8_t>& payloadBuf);
    void SetMsgId(const uint64_t msgId);
    std::vector<uint8_t> Encode();
    
    uint8_t GetVersion() const;
    uint8_t GetCommand() const;
    uint64_t GetPayloadLength() const;
    uint64_t GetMessageId() const;
    MessageType GetMessageType() const;
    uint8_t GetMessageTwoWay() const;
    std::vector<ExtensionHeader_t> GetExtensionHeaders();
    std::vector<uint8_t> GetBodyData() const;
    
private:
    Meta_t meta_;
    std::vector<ExtensionHeader_t> m_headerList;
    std::vector<uint8_t> body_data_;
};
}
