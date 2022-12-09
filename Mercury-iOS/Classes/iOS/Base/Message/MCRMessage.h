//
//  MCRMessage.h
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import <Foundation/Foundation.h>
#import "MCRHeader.h"
#import "MCRResponse.h"

NS_ASSUME_NONNULL_BEGIN

@interface MCRMessage : NSObject

@property (nonatomic, assign) MCRCommand command;

@property (nonatomic, assign) BOOL isTwoWay;

@property (nonatomic, assign) NSInteger type;

@property (nonatomic, assign) NSInteger code;

@property (nonatomic, strong, nullable) NSData *body;

@property (nonatomic, strong, nullable) NSArray<MCRHeader *> *headers;

+ (instancetype)twoWayRequest;

- (NSString *)description;

@end

NS_ASSUME_NONNULL_END
