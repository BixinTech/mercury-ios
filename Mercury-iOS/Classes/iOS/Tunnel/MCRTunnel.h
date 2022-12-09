//
//  MCRTunnel.h
//  Mercury
//
//  Created by 姜腾 on 2019/10/19.
//

#import <Foundation/Foundation.h>
#import "MCRSocketAddress.h"
#import "MCRMercury.h"

NS_ASSUME_NONNULL_BEGIN

@class MCRTunnel, MCRMessage, MCRResponse;

@protocol MCRTunnelDelegate <NSObject>
@optional

- (void)onReadPackage:(MCRTunnel*)tunnel package:(MCRMessage*)message error:(NSError *)error;
- (void)onTunnelStatus:(MercuryStatus)status;
- (void)pvWithCommandCode:(NSInteger )commandCode code:(NSInteger)code responseTime:(NSInteger)time uploadSample:(float)uploadSample;

- (NSInteger)tunnelType;
@end

@interface MCRTunnel : NSObject
- (instancetype)initWithDelegate:(id<MCRTunnelDelegate>)aDelegate
                  completedQueue:(nullable dispatch_queue_t)completedQueue;

- (void)start;

- (void)background:(BOOL)isBackground;
- (void)sendMessage:(MCRMessage *)msg;

- (void)sendMessage:(MCRMessage *)msg completion:(MCRRespCallback _Nullable)completion;

- (void)updateConfig:(NSDictionary *)diction;

- (NSTimeInterval)timeFromLastMessage;
- (void)stop;
@end

NS_ASSUME_NONNULL_END
