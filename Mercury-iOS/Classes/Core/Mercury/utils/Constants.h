//
//  Constants.h
//  Pods
//
//  Created by Ju Liaoyuan on 2022/11/2.
//

#ifndef Constants_h
#define Constants_h

enum class MessageFlag: uint8_t
{
    PING = 0,
    CUSTOM = 1,
};

enum class MessageType: uint8_t
{
    RESP = 0,
    REQ = 1,
};

#endif /* Constants_h */
