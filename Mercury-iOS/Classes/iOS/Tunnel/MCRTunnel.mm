//
//  MCRTunnel.m
//  Mercury
//
//  Created by 姜腾 on 2019/10/19.
//

#import "MCRTunnel.h"
#include "Tunnel.h"
#import "MCRTunnelCallBack.h"
#import "MCRMessage.h"
#import "MCRMessage+MercuryMessage.h"
#import "MCRConfigurator.h"
#import "MCRLog.h"
#include "TunnelDefine.h"
#import "MCRSocketConfigTools.h"
#import <memory>
#import <thread>
#import "MCRResponse.h"
#import "MCRResponse+Data.h"
#import "MCRMessageReceipt.h"
#import "MCRConstants.h"
#import "MCRConfigurator.h"

using namespace mercury;

static inline NSTimeInterval currentTimestamp() {
    return [[NSDate date] timeIntervalSince1970] * 1000;
}

static inline NSError * timeout() {
    return [NSError errorWithDomain:MCRErrorDomain
                               code:MCRErrorCodeTimeout
                           userInfo:@{NSLocalizedDescriptionKey: @"mercury message response timeout"}];
}

@interface MCRTunnel (){
    Tunnel *_tunnel;
    MCRTunnelCallBack *_callBack;
    std::mutex _lock;
}


@property (nonatomic, weak) id<MCRTunnelDelegate> delegate;

@property (nonatomic, strong) dispatch_queue_t completedQueue;

@property (nonatomic, strong) NSMutableDictionary<NSNumber *, MCRMessageReceipt *> *callbacks;

@property (nonatomic, strong) NSTimer *timeoutTimer;

@end

@implementation MCRTunnel

- (instancetype)initWithDelegate:(id<MCRTunnelDelegate>)aDelegate
                  completedQueue:(nullable dispatch_queue_t)completedQueue {
    if (self = [super init]) {
        _delegate = aDelegate;
        _completedQueue = completedQueue;
        _callbacks = @{}.mutableCopy;
        if (_completedQueue == nil) {
            _completedQueue = dispatch_get_main_queue();
        }
        self->_tunnel = new Tunnel();
        self->_callBack = new MCRTunnelCallBack();
        [self setupTunnelCallback];
        
        self->_tunnel->SetCallback(self->_callBack);
        [self setupTimeoutTimer];
    }
    return self;
}

- (void)setupTunnelCallback {

    __weak typeof(self) _weakSelf = self;
    self->_callBack->SetRecieveCallback([=](TunnelMessage message){
        __strong typeof(_weakSelf) _strongSelf = _weakSelf;
        dispatch_async(_strongSelf.completedQueue, ^{
            __strong typeof(_weakSelf) _strongSelf = _weakSelf;
            [_strongSelf onMessage:message];
        });
    });
    
    self->_callBack->SetNetworkStatusCallback([=](NetStatus status){
        __strong typeof(_weakSelf) _strongSelf = _weakSelf;
        dispatch_async(_strongSelf.completedQueue, ^{
            [_strongSelf onTunnelStatus:status];
        });
    });
    self->_callBack->SetLogCallback([=](std::string log){
        __strong typeof(_weakSelf) _strongSelf = _weakSelf;
        dispatch_async(_strongSelf.completedQueue, ^{
            [_strongSelf onLog:log];
        });
    });
    self->_callBack->SetLoginCallback([=](LoginInfo& info){
        __strong typeof(_weakSelf) _strongSelf = _weakSelf;

        MCRConfigurator *config = [MCRConfigurator configurator];
        info.platform = 1;
        info.appId = (uint8_t)config.appId;
        info.systemVersion = [MCRConfigurator systemVersion].UTF8String;
        info.appVersion =  [MCRConfigurator appVersion].UTF8String;
        NSString *bd = [config bundleId];
        if (bd.length) {
            info.bundleId = bd.UTF8String;
        }
        NSString *d = [config unionId];
        if (d.length) {
            info.deviceId = d.UTF8String;
        }
        NSString *m = [MCRConfigurator platformString];
        if (m.length) {
            info.model = m.UTF8String;
        }
        NSString *u = [config userId];
        if (u.length) {
           info.userId = u.UTF8String;
        }
        if ([_strongSelf.delegate respondsToSelector:@selector(tunnelType)]) {
            info.type = (int)[_strongSelf.delegate tunnelType];
        }else {
            info.type = 0;
        }
    });
    
    self->_callBack->SetAddressCallback([=](Address& address){
        MCRSocketAddress *maddr = [MCRConfigurator configurator].address;
        address.host = maddr.host.UTF8String;
        address.port = [NSString stringWithFormat:@"%u",(unsigned int)maddr.port].UTF8String;
    });
    
    self->_callBack->SetHandStatisticsCallback([=](TunnelStatisticsResult result) {
        __strong typeof(_weakSelf) _strongSelf = _weakSelf;
        NSInteger commandCode = (NSInteger )result.type_;
        [_strongSelf pvWithCommandCode:commandCode code:result.errorCode_ responseTime:result.timeUsing_ uploadSample:result.reportRate_];
    });
}

- (void)start {
    self->_tunnel->Start();
}

- (void)stop {
    self->_tunnel->Stop();
}

- (void)background:(BOOL)isBackground {
    self->_tunnel->OnBackGround(isBackground);}

- (NSTimeInterval)timeFromLastMessage {
    uint64_t time = self->_tunnel->GetTimeElapseFromLastMessage();
    NSTimeInterval s = time * 0.001;
    return s;
}
- (void)updateConfig:(NSDictionary *)diction {
    self->_tunnel->UpdateConfig([MCRSocketConfigTools socketConfigFor:diction]);
    MCRLOG(@"update config:%@",diction);
}

- (void)sendMessage:(MCRMessage *)msg {
    [self sendMessage:msg completion:nil];
}

- (void)sendMessage:(MCRMessage *)msg completion:(MCRRespCallback)completion {
    mercury::TunnelMessage sendMsg = [msg cMessage];
    sendMsg.msgId = MessageIdGenerator::instance().Gen_msg_id();
    if (completion != nil && sendMsg.type == MessageType::REQ && sendMsg.twoWay == 1) {
        std::lock_guard<std::mutex> lg(_lock);
        MCRMessageReceipt *receipt = [MCRMessageReceipt new];
        receipt.messageId = sendMsg.msgId;
        receipt.callback = completion;
        receipt.timestamp = currentTimestamp();
        [self.callbacks setObject:receipt forKey:@(sendMsg.msgId)];
    }
    self->_tunnel->Send(sendMsg);
}

#pragma mark tunnel call back
- (void)onMessage:(TunnelMessage)message {
    NSError *error;
    MCRMessage *m = [MCRMessage messageWithCMessage:message];
    if (message.msgId != 0 && message.type == MessageType::RESP) {
        /// two way 方式
        std::lock_guard<std::mutex> lg(_lock);
        MCRMessageReceipt *receipt = self.callbacks[@(message.msgId)];
        if (receipt && receipt.callback) {
            NSError *error = nil;
            MCRResponse *resp = [MCRResponse responseFromData:m.body error:&error];
            receipt.callback(resp, error);
            [self.callbacks removeObjectForKey:@(message.msgId)];
            return;
        }
    }
    if (message.type == MessageType::REQ) {
        /// 非 two way
        if ([self.delegate respondsToSelector:@selector(onReadPackage:package:error:)]) {
            [self.delegate onReadPackage:self package:m error:error];
        }
    }
}

- (void)onTunnelStatus:(NetStatus)status {
    NSString *s = @"";
    MercuryStatus tunnelstatus = MercuryStatusDisconnect;
    switch (status) {
        case mercury::NetStatus::connecting:
            s = @"connecting";
            tunnelstatus = MercuryStatusConnecting;
            break;
        case mercury::NetStatus::connected:
            s = @"connected";
            tunnelstatus = MercuryStatusConnected;
            break;
        case mercury::NetStatus::disconnect:
            s = @"disconnect";
            break;
        default:
            s = [NSString stringWithFormat:@"unknown %d",status];
            break;
    }
    
    MCRLOG(@"status [%@]",s);
    if ([self.delegate respondsToSelector:@selector(onTunnelStatus:)]) {
        [self.delegate onTunnelStatus:tunnelstatus];
    }
}

- (void)onLog:(std::string)log {
    NSString *s = [NSString stringWithCString:log.c_str() encoding:[NSString defaultCStringEncoding]];
    MCRLOG(@"%@",s);
}

- (void)pvWithCommandCode:(NSInteger )commandCode code:(NSInteger)code responseTime:(NSInteger)time uploadSample:(float)uploadSample{
    if ([self.delegate respondsToSelector:@selector(pvWithCommandCode:code:responseTime:uploadSample:)]) {
        [self.delegate pvWithCommandCode:commandCode code:code responseTime:time uploadSample:uploadSample];
    }
}

- (void)setupTimeoutTimer {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.timeoutTimer) {
            [self.timeoutTimer invalidate];
            self.timeoutTimer = nil;
        }
        __weak typeof(self) _weakSelf = self;
        self.timeoutTimer = [NSTimer scheduledTimerWithTimeInterval:3 repeats:true block:^(NSTimer * _Nonnull timer) {
            __strong typeof(_weakSelf) _strongSelf = _weakSelf;
            [_strongSelf checkTimeoutMessage];
        }];
    });
}

- (void)checkTimeoutMessage {
    std::lock_guard<std::mutex> lg(_lock);
    NSTimeInterval current = currentTimestamp();
    NSMutableArray *needRemoveRecipts = @[].mutableCopy;
    for (MCRMessageReceipt *receipt in self.callbacks.allValues) {
        if (current - receipt.timestamp > 3000) {
            if (receipt.callback) {
                receipt.callback(nil, timeout());
            }
            [needRemoveRecipts addObject:@(receipt.messageId)];
        }
    }
    [self.callbacks removeObjectsForKeys:needRemoveRecipts];
}

- (void)dealloc {
    if (_timeoutTimer) {
        [_timeoutTimer invalidate];
        _timeoutTimer = nil;
    }
    delete (_tunnel);
    delete (_callBack);
}
@end

