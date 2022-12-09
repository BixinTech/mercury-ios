//
//  MercuryService.h
//  MercuryService
//
//  Created by 姜腾 on 2019/11/20.
//

#import <Foundation/Foundation.h>
#import <MCRMercury.h>
#import "MCRResponse.h"
#import "MCRIMService.h"

NS_ASSUME_NONNULL_BEGIN

@protocol MercuryServiceDelegate <NSObject>

@optional

/** 处理消息
 @param package 消息包
 */
- (void)onHandlePackage:(MCRMessage *)package;

/** Mercury 通道状态变化
 @param status 通道状态，@see MercuryStatus
 */
- (void)onTunnelStatus:(MercuryStatus)status;

- (NSInteger)tunnelType;

@end

@protocol MCRPushDelegate <NSObject>

- (void)receivePushData:(NSData *)data;

@end

typedef void (^MCRPushDataHandler)(NSData *data);

@interface MercuryService : NSObject
@property (nonatomic, strong) MCRIMService *imService;
@property (nonatomic, weak) id<MercuryServiceDelegate> delegate;

/// 处理push消息，主线程回调
@property (nullable,nonatomic,copy) MCRPushDataHandler pushDataHandler;

+ (MercuryService *)shareInstance;

- (void)startService;

- (void)stopService;

- (void)resetConnect;

- (void)sendMessage:(MCRMessage *)msg;

- (void)sendMessage:(MCRMessage *)msg completion:(MCRRespCallback _Nullable)completion;

- (void)addEachMessage:(MCRMessage *)message type:(NSString *)type cancel:(BOOL)cancel;

- (BOOL)isConnected;

- (BOOL)isConnecting;

- (BOOL)isDisconnect;

- (void)addMCRPushDelegate:(id<MCRPushDelegate>)delegate;

- (void)removeMCRPushDelegate:(id<MCRPushDelegate>)delegate;

/// 业务方需求追加 只对onTunnelStatus 生效
- (void)addMercuryServiceDelegate:(id<MercuryServiceDelegate>)delegate;

- (void)removeMercuryServiceDelegate:(id<MercuryServiceDelegate>)delegate;

@end

NS_ASSUME_NONNULL_END


