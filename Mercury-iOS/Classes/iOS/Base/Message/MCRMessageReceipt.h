//
//  MCRMessageReceipt.h
//  MercuryService
//
//  Created by Ju Liaoyuan on 2022/11/3.
//

#import <Foundation/Foundation.h>
#import "MCRMessage.h"

NS_ASSUME_NONNULL_BEGIN

@interface MCRMessageReceipt : NSObject

@property (nonatomic, assign) uint64_t messageId;

@property (nonatomic, assign) NSTimeInterval timestamp;

@property (nonatomic, copy) MCRRespCallback callback;

@end

NS_ASSUME_NONNULL_END
