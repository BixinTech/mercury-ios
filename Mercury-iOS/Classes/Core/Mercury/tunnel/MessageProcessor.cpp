#include "MessageProcessor.h"
#include <string>
#include "json.h"
#include "zlib.h"

namespace mercury
{
MessageProcessor::MessageProcessor(const MessageProtocol& msg)
:msg_(msg), uncompressedBuffer_(RESERVE_UNCPMPRESS_DATA_SIZE)
{
}

MessageProcessor::~MessageProcessor()
{
    
}

void MessageProcessor::Process()
{
    process_header(msg_.GetExtensionHeaders());
    process_body(msg_.GetBodyData());
}

uint8_t MessageProcessor::GetMessageCommand() const
{
    return msg_.GetCommand();
}

uint64_t MessageProcessor::GetMessageId() const
{
    return msg_.GetMessageId();
}

MessageType MessageProcessor::GetMessageType() const
{
    return msg_.GetMessageType();
}

uint8_t MessageProcessor::GetMessageTwoWay() const
{
    return msg_.GetMessageTwoWay();
}

std::vector<std::string> MessageProcessor::GetAckMessageIds() const
{
    return ackMessageId_;
}

std::vector<mercury::ExtensionHeader_t> MessageProcessor::GetMessageHeaders() const
{
    return buisinessMessageHeaders_;
}

std::vector<uint8_t> MessageProcessor::GetMessageBodyData() const
{
    return msgBodyData_;
}

void MessageProcessor::process_header(const std::vector<ExtensionHeader_t>& headers)
{
    for (const auto& header : headers)
    {
        if (MessageHeaderType::compressed == (MessageHeaderType)header.headerType)
        {
            auto compressType = std::string(header.headerData.begin(), header.headerData.end());
            if ("1" == compressType)
            {
                compressType_ = CompressType::zip;
            }
            continue;
        } else {
            buisinessMessageHeaders_.push_back(header);
        }
    }
}

void MessageProcessor::process_body(const std::vector<uint8_t>& msgData)
{
    msgBodyData_ = uncompressed_body(msgData);
}

std::vector<uint8_t> MessageProcessor::uncompressed_body(const std::vector<uint8_t>& body)
{
    if (CompressType::zip == compressType_)
    {
        unsigned long uncompressedDataSize = uncompressedBuffer_.size();
        while (true)
        {
            auto ret = uncompress((Bytef *)uncompressedBuffer_.data(), &uncompressedDataSize, (Bytef *)body.data(), body.size());
            if (Z_OK == ret)
            {
                uncompressedBuffer_.resize(uncompressedDataSize);
                return uncompressedBuffer_;
            }
            else if (Z_BUF_ERROR == ret)
            {
                //输出buffer太小，增加输出buffer大小
                uncompressedDataSize *= 2;
                if (uncompressedDataSize > MAX_UNCPMPRESS_DATA_SIZE)
                {
                    //输出buffer大于51M，退出，直接返回原数据
                    break;
                }
                uncompressedBuffer_.resize(uncompressedDataSize);
            }
            else
            {
                //其他错误，直接返回原数据
                break;
            }
        }
    }
    return body;
}

}
