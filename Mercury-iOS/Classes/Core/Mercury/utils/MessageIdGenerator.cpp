//
//  MessageIdGenerator.cpp
//  SonaRoom
//
//  Created by Ju Liaoyuan on 2022/11/9.
//

#include "MessageIdGenerator.h"


namespace mercury {
uint64_t MessageIdGenerator::Gen_msg_id() {
    return messageId_++;
}
void MessageIdGenerator::Reset_msg_id() {
    messageId_ = 1;
}
}
