//
//  MCRConfigurator.h
//  Mercury
//
//  Created by 姜腾 on 2019/11/7.
//

#import <Foundation/Foundation.h>
#import "MCRSocketAddress.h"

typedef NSString *_Nullable(^MCRConfigStringBlock)(void);

typedef MCRSocketAddress *_Nullable(^MCRAddressConfigBlock)(void);

NS_ASSUME_NONNULL_BEGIN

@interface MCRConfigurator : NSObject

@property (nonatomic, assign) NSInteger appId;

+ (MCRConfigurator *)configurator;

- (nullable NSString *)unionId;

- (nullable NSString *)userId;

- (nullable NSString *)bundleId;

- (nullable MCRSocketAddress *)address;

- (NSTimeInterval)sendMsgTimeoutInterval;

/** 设置userid
 */
- (void)setupUserId:(MCRConfigStringBlock)block;


/** 设置 Unid
 */
- (void)setupUnionid:(MCRConfigStringBlock)block;

/** 设置 BundleId
 */
- (void)setupBundleId:(MCRConfigStringBlock)block;

/** 设置长连地址
 */
- (void)setupAddress:(MCRAddressConfigBlock)block;

/** 设置消息发送时的超时时间
 @param interval 超时时间，**单位ms**，默认时间为 300 ms
 */
- (void)setupSendMsgTimeoutInterval:(NSTimeInterval)interval;


+ (NSString *)systemVersion;
+ (NSString *)appVersion;
+ (NSString *)platformString;
@end

NS_ASSUME_NONNULL_END
