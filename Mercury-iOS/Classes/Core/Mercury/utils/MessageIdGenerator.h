//
//  MessageIdGenerator.h
//  SonaRoom
//
//  Created by Ju Liaoyuan on 2022/11/9.
//

#pragma once
#include <cstdint>

namespace mercury {

class MessageIdGenerator
{
public:
    
    MessageIdGenerator(const MessageIdGenerator&)=delete;
    MessageIdGenerator operator=(const MessageIdGenerator&)=delete;
    static MessageIdGenerator& instance() {
        static MessageIdGenerator instance;
        return instance;
    }
    uint64_t Gen_msg_id();
    void Reset_msg_id();
private:
    MessageIdGenerator() {};
    uint64_t messageId_ = 1;
};
}
