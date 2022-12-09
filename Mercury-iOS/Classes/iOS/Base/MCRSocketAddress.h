//
//  MCRSocketAddress.h
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MCRSocketAddress : NSObject
@property (nonatomic, copy) NSString *host;
@property (nonatomic) UInt32 port;

+ (instancetype)addressWithHost:(NSString *)host andPort:(UInt32)port;

- (NSString *)description;
@end

NS_ASSUME_NONNULL_END
