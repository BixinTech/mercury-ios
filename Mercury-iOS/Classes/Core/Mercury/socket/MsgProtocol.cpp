#include "MsgProtocol.h"
#include <cstdint>
#include <string>
#include "../utils/VarInt.h"
#include "../utils/Conversion.h"
#include "../utils/Constants.h"

namespace mercury {
MessageProtocol::MessageProtocol()
{
}

MessageProtocol::MessageProtocol(const MessageProtocol& right)
{
    meta_ =      right.meta_;
    m_headerList = right.m_headerList;
    body_data_ = right.body_data_;
}

MessageProtocol::~MessageProtocol()
{
}

uint8_t MessageProtocol::GetVersion() const
{
    return meta_.fixedInfo.version;
}

uint8_t MessageProtocol::GetCommand() const
{
    return meta_.command;
}

uint64_t MessageProtocol::GetPayloadLength() const
{
    return meta_.payloadLength;
}

uint64_t MessageProtocol::GetMessageId() const
{
    return meta_.msgId;
}

MessageType MessageProtocol::GetMessageType() const
{
    return meta_.fixedInfo.type;
}

uint8_t MessageProtocol::GetMessageTwoWay() const
{
    return meta_.fixedInfo.twoWay;
}


void MessageProtocol::AddExtensionHeader(uint8_t type, const std::vector<uint8_t>& headerData)
{
    ExtensionHeader_t extHeader;
    extHeader.headerType = type;
    extHeader.headerData = headerData;
    m_headerList.push_back(extHeader);
}

void MessageProtocol::SetBodyData(const std::vector<uint8_t>& bodyData)
{
    body_data_ = bodyData;
}

std::vector<uint8_t> MessageProtocol::Encode()
{
    
    if (meta_.fixedInfo.heartbeat == 1)
    {
        std::vector<uint8_t> pingBuf;
        pingBuf.push_back((uint8_t)meta_.fixedInfo.type);
        pingBuf.push_back(meta_.fixedInfo.twoWay);
        pingBuf.push_back(meta_.fixedInfo.heartbeat);
        pingBuf.push_back(meta_.fixedInfo.version);
        pingBuf.push_back(meta_.msgId);
        return pingBuf;
    }
    
    // meta_fixed_info (type + twoway + hearbeat flag + version)
    std::vector<uint8_t> messageDataBuf;
    Meta_Fixed_t fixed_info;
    fixed_info.type = meta_.fixedInfo.type;
    fixed_info.twoWay = meta_.fixedInfo.twoWay;
    fixed_info.heartbeat = meta_.fixedInfo.heartbeat;
    fixed_info.version = meta_.fixedInfo.version;
    messageDataBuf.insert(messageDataBuf.end(), (uint8_t *)&fixed_info, (uint8_t *)&fixed_info + sizeof(fixed_info));
    
    // message id
    uint8_t msgIdBuf[10] = { 0 };
    auto msgIdLen = encodeVarint(meta_.msgId, msgIdBuf);
    messageDataBuf.insert(messageDataBuf.end(), msgIdBuf, msgIdBuf + msgIdLen);
    
    // command
    messageDataBuf.push_back(meta_.command);
    
    // header
    std::vector<uint8_t> extHeaderBuf;
    for (size_t i=0; i< m_headerList.size(); ++i)
    {
        uint8_t headerType= m_headerList[i].headerType;
        extHeaderBuf.insert(extHeaderBuf.end(), { headerType });
        uint8_t varHeaderLenBuf[10] = { 0 };
        auto sizeLen = encodeVarint(m_headerList[i].headerData.size(), varHeaderLenBuf);
        extHeaderBuf.insert(extHeaderBuf.end(), varHeaderLenBuf, varHeaderLenBuf + sizeLen);
        extHeaderBuf.insert(extHeaderBuf.end(), m_headerList[i].headerData.begin(), m_headerList[i].headerData.end());
    }
    
    // calcu payload length
    uint64_t payloadTotalLen = extHeaderBuf.size() + body_data_.size();
    uint8_t payloadLenBuf[10] = { 0 };
    // encode to varint
    auto payloadLen = encodeVarint(payloadTotalLen, payloadLenBuf);
    messageDataBuf.insert(messageDataBuf.end(), payloadLenBuf, payloadLenBuf + payloadLen);
    
    // insert header size
    messageDataBuf.insert(messageDataBuf.end(), m_headerList.size());
    // insert header
    messageDataBuf.insert(messageDataBuf.end(), extHeaderBuf.begin(), extHeaderBuf.end());
    // insert body
    messageDataBuf.insert(messageDataBuf.end(), body_data_.begin(), body_data_.end());
    
    return messageDataBuf;
}

void MessageProtocol::SetMeta(const Meta_t& meta)
{
    meta_ = meta;
}

void MessageProtocol::SetPayload(const std::vector<uint8_t>& payloadBuf)
{
    if (payloadBuf.empty()) {
        return;
    }
    uint8_t headerSize = payloadBuf[0];
    size_t headerBound = 0;
    
    for (size_t i = 1; i < payloadBuf.size();)
    {
        if (headerSize <= 0) {
            headerBound = i;
            break;
        }
        ExtensionHeader_t extHeader;
        extHeader.headerType = payloadBuf[i];
        i += sizeof(ExtensionHeader_t::headerType);
        size_t inputSize = 0;
        size_t headerLength = (size_t)decodeVarint(&payloadBuf[i], inputSize);
        i += inputSize;
        
        extHeader.headerData = std::vector<uint8_t>(payloadBuf.begin() + i, payloadBuf.begin() + i + headerLength);
        i += headerLength;
        m_headerList.push_back(extHeader);
        headerSize -= 1;
    }
    body_data_ = std::vector<uint8_t>(payloadBuf.begin() + headerBound, payloadBuf.end());
}

std::vector<ExtensionHeader_t> MessageProtocol::GetExtensionHeaders()
{
    return m_headerList;
}

std::vector<uint8_t> MessageProtocol::GetBodyData() const
{
    return body_data_;
}
}
