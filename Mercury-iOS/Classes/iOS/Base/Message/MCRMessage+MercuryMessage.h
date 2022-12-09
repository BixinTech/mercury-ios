//
//  MCRMessage+MercuryMessage.h
//  Mercury
//
//  Created by 姜腾 on 2019/11/7.
//

#import <Foundation/Foundation.h>
#import "MCRMessage.h"
#include "TunnelDefine.h"
NS_ASSUME_NONNULL_BEGIN

@interface MCRMessage (MercuryMessage)

+ (instancetype)messageWithCMessage:(mercury::TunnelMessage)msg;

- (mercury::TunnelMessage)cMessage;

@end

NS_ASSUME_NONNULL_END
