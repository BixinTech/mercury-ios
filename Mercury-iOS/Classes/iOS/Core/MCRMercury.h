//
//  MCRMercury.h
//  Mercury
//
//  Created by 姜腾 on 2019/11/7.
//

#import <Foundation/Foundation.h>
#import "MCRMessage.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger,MercuryStatus){
    MercuryStatusConnecting    = 0,
    MercuryStatusConnected     = 1,
    MercuryStatusDisconnect    = 2
};

@class MCRMercury,MCRMessage,MCRSocketAddress;
@protocol MCRMercuryDelegate <NSObject>
@optional
- (void)onReadPackage:(MCRMessage *)package mercury:(MCRMercury *)mercury error:(NSError *)error;
- (void)onTunnelStatus:(MercuryStatus)status;
- (MCRSocketAddress *)getAddress;

- (void)pvWithCommandCode:(NSInteger )commandCode code:(NSInteger)code responseTime:(NSInteger)time uploadSample:(float)uploadSample;
- (NSInteger)tunnelType;
@end

@interface MCRMercury : NSObject
- (instancetype)initWithDelegate:(id<MCRMercuryDelegate>)aDelegate
                  completedQueue:(nullable dispatch_queue_t)completedQueue;

- (void)updateConfig:(NSDictionary *)diction;

- (void)startService;

- (void)stopService;

- (void)background:(BOOL)isBackground;

- (void)sendMessage:(MCRMessage *)msg;

- (void)sendMessage:(MCRMessage *)msg completion:(MCRRespCallback)completion;

- (NSTimeInterval)timeFromLastMessage;
@end

NS_ASSUME_NONNULL_END
