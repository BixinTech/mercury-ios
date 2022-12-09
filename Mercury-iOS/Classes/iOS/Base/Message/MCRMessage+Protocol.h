//
//  MCRMessage+Protocol.h
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import <Foundation/Foundation.h>
#include "MsgProtocol.h"
#import "MCRMessage.h"

NS_ASSUME_NONNULL_BEGIN

@interface MCRMessage (Protocol)

+ (instancetype)messageWithProtocol:(mercury::MessageProtocol)protocol;
//- (mercury::MessageProtocol)messageProtocol;

@end

NS_ASSUME_NONNULL_END
