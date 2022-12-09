//
//  MCRMercury.m
//  Mercury
//
//  Created by 姜腾 on 2019/11/7.
//

#import "MCRMercury.h"
#import "MCRTunnel.h"
#import "MCRMessage.h"
#import "MCRLog.h"

@interface MCRMercury()<MCRTunnelDelegate>
@property (nonatomic, strong) MCRTunnel *tunnel;
@property (nonatomic, weak) id<MCRMercuryDelegate> delegate;
@property (nonatomic, strong) dispatch_queue_t completedQueue;
@end

@implementation MCRMercury

- (instancetype)initWithDelegate:(id<MCRMercuryDelegate>)aDelegate
                  completedQueue:(nullable dispatch_queue_t)completedQueue {
    if (self = [super init]) {
        _delegate = aDelegate;
        _completedQueue = completedQueue;
        _tunnel = [[MCRTunnel alloc] initWithDelegate:self completedQueue:completedQueue];
    }
    return self;
}
- (void)startService {
    MCRLOG(@"start service");
    [self.tunnel start];
}

- (void)stopService {
    [self.tunnel stop];
}
- (void)background:(BOOL)isBackground {
    [self.tunnel background:isBackground];
}
- (void)sendMessage:(MCRMessage *)msg {
    [self.tunnel sendMessage:msg];
}

- (void)sendMessage:(MCRMessage *)msg completion:(MCRRespCallback)completion {
    [self.tunnel sendMessage:msg completion:completion];
}

- (void)updateConfig:(NSDictionary *)diction {
    [self.tunnel updateConfig:diction];
}
#pragma mark - tunnel delegate

- (void)onReadPackage:(MCRTunnel *)tunnel package:(MCRMessage *)message error:(NSError *)error {
    if ([self.delegate respondsToSelector:@selector(onReadPackage:mercury:error:)]) {
        [self.delegate onReadPackage:message mercury:self error:error];
    }
}

- (void)onTunnelStatus:(MercuryStatus)status {
    if ([self.delegate respondsToSelector:@selector(onTunnelStatus:)]) {
        [self.delegate onTunnelStatus:status];
    }
}

- (NSTimeInterval)timeFromLastMessage {
    return [self.tunnel timeFromLastMessage];
}

- (NSInteger)tunnelType {
    if ([self.delegate respondsToSelector:@selector(tunnelType)]) {
        return [self.delegate tunnelType];
    }
    return 0;
}

- (void)pvWithCommandCode:(NSInteger )commandCode code:(NSInteger)code responseTime:(NSInteger)time uploadSample:(float)uploadSample {
    if ([self.delegate respondsToSelector:@selector(pvWithCommandCode:code:responseTime:uploadSample:)]) {
        [self.delegate pvWithCommandCode:commandCode code:code responseTime:time uploadSample:uploadSample];
    }
}
@end
