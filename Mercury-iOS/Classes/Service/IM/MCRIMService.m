//
//  MCRIMService.m
//  MercuryService
//
//  Created by 姜腾 on 2020/4/16.
//

#import "MCRIMService.h"
#import "MercuryHeader.h"
#import "NSDictionary+Mercury.h"
#import "MCRRoomService.h"

@interface MCRIMService()
@property (nonatomic, strong) NSMutableArray *delegateMutableArray;
@end

@implementation MCRIMService

- (id)init {
    if (self = [super init]) {
        _delegateMutableArray = [[NSMutableArray alloc] init];
    }
    return self;
}

- (BOOL)canHandleCommand:(NSInteger)command {
    BOOL b = NO;
    if ((120 <= command && command <= 139) || command == 199) {
        b = YES;
    }
    return b;
}

- (void)receiveIMMessage:(MCRMessage *)package {
    NSInteger c = package.command;
    switch (c) {
        case 124:
            [self handleGroupMessage:package];
            break;
        case 125:
            [self handleReceiptMessage:package];
            break;
        case 126:
            [self handleSyncSessionInfoDataMessage:package];
            break;
        //实时消息
        case 123:
            [self handleSyncRealTimeIMMessage:package];
            break;
        //实时消息
        case 127:
            [self handleSyncRealACKMessage:package];
            break;
        default:
            break;
    }
}

- (void)handleGroupMessage:(MCRMessage *)package {
    NSArray *s = nil;
    @synchronized (self) {
        s = [self.delegateMutableArray copy];
    }
    
    NSString *groupid = [self groupidForHeaders:package.headers];
    for (id<MCRIMServiceDelegate> delegate in s) {
        if ([delegate respondsToSelector:@selector(onGroupMessage:groupid:)]) {
            [delegate onGroupMessage:package.body groupid:groupid];
        }
    }
}

- (void)handleReceiptMessage:(MCRMessage *)package {
    NSArray *s = nil;
    @synchronized (self) {
        s = [self.delegateMutableArray copy];
    }
    for (id<MCRIMServiceDelegate> delegate in s) {
        if ([delegate respondsToSelector:@selector(handleReceiptMessage:receiptData:)]) {
            if (package.body) {
                [delegate handleReceiptMessage:1 receiptData:package.body];
            }else{
                [delegate handleReceiptMessage:0 receiptData:package.body];
            }
        }
    }
}

// 返回离线消息
- (void)handleSyncSessionInfoDataMessage:(MCRMessage *)package {
    NSArray *s = nil;
    @synchronized (self) {
        s = [self.delegateMutableArray copy];
    }
    for (id<MCRIMServiceDelegate> delegate in s) {
        if ([delegate respondsToSelector:@selector(syncSessionInfoCode:requestSyncData:)]) {
            if (package.body) {
                [delegate syncSessionInfoCode:1 requestSyncData:package.body];
            }else{
                [delegate syncSessionInfoCode:0 requestSyncData:package.body];
            }
        }
    }
}

// 返回实时消息
- (void)handleSyncRealTimeIMMessage:(MCRMessage *)package {
    NSArray *s = nil;
    @synchronized (self) {
        s = [self.delegateMutableArray copy];
    }
    for (id<MCRIMServiceDelegate> delegate in s) {
        if ([delegate respondsToSelector:@selector(handleSyncRealTimeIMMessage:requestSyncData:)]) {
            if (package.body) {
                [delegate handleSyncRealTimeIMMessage:1 requestSyncData:package.body];
            }else{
                [delegate handleSyncRealTimeIMMessage:0 requestSyncData:package.body];
            }
        }
    }
}

// 返回ACK消息
- (void)handleSyncRealACKMessage:(MCRMessage *)package {
    NSArray *s = nil;
    @synchronized (self) {
        s = [self.delegateMutableArray copy];
    }
    for (id<MCRIMServiceDelegate> delegate in s) {
        if ([delegate respondsToSelector:@selector(handleSyncRealACKMessage:requestSyncData:)]) {
            if (package.body) {
                [delegate handleSyncRealACKMessage:1 requestSyncData:package.body];
            }else{
                [delegate handleSyncRealACKMessage:0 requestSyncData:package.body];
            }
        }
    }
}


- (void)sendGroupMessage:(NSData *)messageData groupid:(NSString *)groupid {
    MCRMessage *model = [[MCRMessage alloc] init];
    model.command = MCRCommandSendGroupMessage;
    model.body = messageData;
    model.headers = [self groupHeadersForRoomid:groupid];
    
    [[MercuryService shareInstance] sendMessage:model];
}

- (void)addDelegate:(id<MCRIMServiceDelegate>)delegate {
    @synchronized (self) {
        [self.delegateMutableArray addObject:delegate];
    }
}

- (void)removeDelegate:(id<MCRIMServiceDelegate>)delegate {
    @synchronized (self) {
        [self.delegateMutableArray removeObject:delegate];
    }
}

- (NSArray<MCRHeader *> *)groupHeadersForRoomid:(NSString *)groupid {
    MCRHeader *h = [[MCRHeader alloc] init];
    h.type = MCRHeaderTypeRoom;
    NSData *d = [NSJSONSerialization dataWithJSONObject:@{@"room":groupid} options:0 error:NULL];
    h.data = d;
    return @[h];
}

- (NSString *)groupidForHeaders:(NSArray<MCRHeader *> *)headers {
    NSString *roomid = @"";
    for (MCRHeader *h in headers) {
        if (h.type == 7) {
            
            NSDictionary *d = [NSDictionary mercuryObjectWithData:h.data];
            roomid = [d mercuryStringForKey:@"groupid"];
            break;
        }
    }
    return roomid ? : @"";
}

@end
