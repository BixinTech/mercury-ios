//
//  MCRoomService.m
//  MercuryService
//
//  Created by 姜腾 on 2020/4/14.
//

#import "MCRRoomService.h"
#import "MercuryHeader.h"
#import "NSDictionary+Mercury.h"
#import "MCREnterRoomModel.h"
#import "NSDictionary+Mercury.h"

@interface MCRRoomService()<MercuryServiceDelegate>

@property (atomic, assign) BOOL needEnter;

@property (nonatomic, weak) MCRRoomSession *session;

@end

@implementation MCRRoomService

+ (MCRRoomService *)shareInstance {
    static MCRRoomService * instance = nil;
    static  dispatch_once_t pred;
    dispatch_once(&pred, ^{
        instance = [[MCRRoomService alloc] init];
    });
    return instance;
}

- (id)init {
    if (self = [super init]) {
        _tunnelServer = [[MercuryService alloc] init];
        _tunnelServer.delegate = self;
        [_tunnelServer startService];
    }
    return self;
}

- (void)enterChatroom:(MCRRoomSession *)session {
    self.session = session;
    [session stateChange:MCRRoomSessionStateEnteringRoom];
    if ([self.tunnelServer isConnected]) {
        [self joinCurrentModel];
    } else if ([self.tunnelServer isDisconnect]) {
        [self.tunnelServer resetConnect];
    }
}

- (void)joinCurrentModel {
    
    if (self.session.status != MCRRoomSessionStateEnteringRoom) {
        return;
    }
    
    MCRMessage *m = [MCRMessage twoWayRequest];
    m.command = MCRCommandEnterRoom;
    
    NSDictionary *dict = @{@"room": self.session.roomId ? : @"",
                           @"uid": self.session.uid ? : @"",
                           @"identity":self.session.isGuest ? @0 : @1,
    };
    
    NSError *error;
    NSData *d = [NSJSONSerialization dataWithJSONObject:dict options:0 error:&error];
    
    if (!error && d.length > 0) {
        // set header
        MCRHeader *header = [[MCRHeader alloc] init];
        header.type = MCRHeaderTypeRoom;
        header.data = d;
        m.headers = @[header];
    }
    
    __weak typeof(self) _weakSelf = self;
    [self.tunnelServer sendMessage:m completion:^(MCRResponse * _Nullable resp, NSError * _Nullable error) {
        __strong typeof(_weakSelf) _strongSelf = _weakSelf;
        [_strongSelf onEnterRoom:resp error:error];
    }];
}

- (void)onEnterRoom:(MCRResponse *)response error:(NSError *)error {
    if (response.code == MCRErrorCodeSuccess) {
        [self.session stateChange:MCRRoomSessionStateInRoom];
    } else if (response.code == MCRErrorCodeTimeout) {
        [self joinTimeout];
    }
    if ([self.session.delegate respondsToSelector:@selector(onEnterRoom:result:session:)]) {
        [self.session.delegate onEnterRoom:self.session.roomId result:error session:self.session];
    }
}

- (void)onEnterRoomTimeout:(nullable NSError *)error {
    if (self.session.status != MCRRoomSessionStateEnteringRoom) {
        return;;
    }
    
    if (error) {
        [self.session stateChange:MCRRoomSessionStateLeaveRoom];
    } else {
        [self.session stateChange:MCRRoomSessionStateInRoom];
    }
    if ([self.session.delegate respondsToSelector:@selector(onEnterRoom:result:session:)]) {
        [self.session.delegate onEnterRoom:self.session.roomId result:error session:self.session];
    }
}

- (void)onTunnelBreak {
    if (self.session.status == MCRRoomSessionStateInRoom) {
        [self.session stateChange:MCRRoomSessionStateLeaveRoom];
    }
}

- (void)exitChatroom:(MCRRoomSession *)session {
    MCRMessage *m = [MCRMessage twoWayRequest];
    m.command = MCRCommandLeaveRoom;
    NSDictionary *dict = @{@"room":session.roomId};
    NSData *d = [NSJSONSerialization dataWithJSONObject:dict options:0 error:NULL];
    if (d) {
        MCRHeader *header = [[MCRHeader alloc] init];
        header.type = MCRHeaderTypeRoom;
        header.data = d;
        [self.tunnelServer sendMessage:m];
    }
}

- (void)sendMessage:(MCRRoomModel *)model {
    [self sendMessage:model completion:nil];
}

- (void)sendMessage:(MCRRoomModel *)model completion:(MCRRespCallback)completion {
    MCRMessage *m = [MCRMessage twoWayRequest];
    m.command = MCRCommandSendRoomMessage;
    m.body = model.data;
    m.headers = [self genRoomHeadersFromModel:model];
    [self.tunnelServer sendMessage:m completion:completion];
}

- (NSArray<MCRHeader *> *)genRoomHeadersFromModel:(MCRRoomModel *)model {
    NSString *roomId = model.roomId ?: @"";
    NSString *ack = model.isAckMsg ? @"1" : @"0";
    MCRHeader *h = [[MCRHeader alloc] init];
    h.type = MCRHeaderTypeRoom;
    NSData *d = [NSJSONSerialization dataWithJSONObject:@{@"room":roomId, @"ack":ack} options:0 error:NULL];
    h.data = d;
    return @[h];
}

- (NSDictionary *)parseRoomHeader:(NSArray<MCRHeader *> *)headers {
    NSDictionary *dic = @{};
    for (MCRHeader *h in headers) {
        if (h.type == MCRHeaderTypeRoom) {
            dic = [NSDictionary mercuryObjectWithData:h.data];
            break;
        }
    }
    return dic;
}


- (void)executeOnMainThread:(void(^)(void))block {
    if (dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL) == dispatch_queue_get_label(dispatch_get_main_queue())) {
        block();
    } else {
        dispatch_async(dispatch_get_main_queue(), block);
    }
}

- (void)joinTimeout {
    NSError *error = [[NSError alloc] initWithDomain:MCRErrorDomain code:-1 userInfo:@{NSLocalizedDescriptionKey:@"timeout"}];
    [self onEnterRoomTimeout:error];
}

#pragma mark - MercuryServiceDelegate

- (void)onTunnelStatus:(MercuryStatus)status {
    if (status == MercuryStatusDisconnect) {
        [self onTunnelBreak];
    } else if (status == MercuryStatusConnected) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self joinCurrentModel];
        });
    }
    if ([self.session.delegate respondsToSelector:@selector(onConnectionStateChanged:session:)]) {
        [self.session.delegate onConnectionStateChanged:status session:self.session];
    }
}
- (void)onHandlePackage:(MCRMessage *)package {
    BOOL canHandle = YES;
    switch (package.command) {
//        case MCRCommandEnterRoom: {
//            // 进入房间成功
//            [self onEnterRoom:package];
//        }
//            break;
//        case MCRCommandLeaveRoom:{
//            // 退出房间
//            [self onLeaveRoom:package];
//        }
//            break;
        case MCRCommandSendRoomMessage:{
            // 普通消息
            [self onReadMessage:package];
        }
            break;
        case MCRCommandSendRoomCmd:{
            // 指令消息
            [self readCMDMessage:package];
        }
            break;
        default:{
            canHandle = NO;
        }
            break;
    }
//    MCRLOG(@"MRS:mercury received message:%@",package.description);
}

- (void)readCMDMessage:(MCRMessage *)package {
    MCRRoomModel *model = [[MCRRoomModel alloc] init];
    model.data = package.body;
    
    NSDictionary *headerDict = [self parseRoomHeader:package.headers];
    model.roomId = headerDict[@"room"];
    MCRSubCommand subCommand = (MCRSubCommand)headerDict[@"signal"];
    
    switch (subCommand) {
        case MCRSubCommandBeKicked:
            [self onKickRoom:model.roomId data:package.body];
            break;
        case MCRSubCommandCloseRoom:
            [self onLeaveRoom:package];
            break;
        default:
        {
            if ([self.session.delegate respondsToSelector:@selector(onNotificationMessage:session:)]) {
                [self.session.delegate onNotificationMessage:model session:self.session];
            }
        }
            break;
    }
}

- (void)onReadMessage:(MCRMessage *)package {
    NSDictionary *headerDict = [self parseRoomHeader:package.headers];
    MCRRoomModel *model = [[MCRRoomModel alloc] init];
    model.data = package.body;
    model.roomId = headerDict[@"room"];
    model.isAckMsg = [headerDict[@"ack"] boolValue];

    if ([self.session.delegate respondsToSelector:@selector(onRoomMessage:session:)]) {
        [self.session.delegate onRoomMessage:model session:self.session];
    }
}

- (void)onKickRoom:(NSString *)roomId data:(NSData *)data {
    [self.session stateChange:MCRRoomSessionStateLeaveRoom];
    if ([self.session.delegate respondsToSelector:@selector(onKickRoom:result:session:)]) {
        [self.session.delegate onKickRoom:roomId result:data session:self.session];
    }
}


- (void)onLeaveRoom:(MCRMessage *)package {
    [self.session stateChange:MCRRoomSessionStateLeaveRoom];
}

- (void)deleteChatroom {
    self.session = nil;
}

- (NSInteger)tunnelType {
    return 2;
}

@end


