//
//  MCRRoomSession.m
//  MercuryService
//
//  Created by 姜腾 on 2020/4/16.
//

#import "MCRRoomSession.h"
#import "MCRRoomService.h"

@implementation MCRRoomSession

- (void)enterRoom {
    [[MCRRoomService shareInstance] enterChatroom:self];
}

- (void)sendMessage:(NSData *)data {
    [self sendMessage:data completion:nil];
}

- (void)sendMessage:(NSData *)data completion:(MCRRespCallback)completion {
    MCRRoomModel *model = [[MCRRoomModel alloc] init];
    model.data = data;
    model.roomId = self.roomId;
    [[MCRRoomService shareInstance] sendMessage:model completion:completion];
}

- (void)sendAckMessage:(NSData *)data {
    MCRRoomModel *model = [[MCRRoomModel alloc] init];
    model.data = data;
    model.isAckMsg = YES;
    model.roomId = self.roomId;
    [[MCRRoomService shareInstance] sendMessage:model];
}

- (void)exitRoom {
    [[MCRRoomService shareInstance] exitChatroom:self];
}

- (void)destoryRoom {
    [[MCRRoomService shareInstance] deleteChatroom];
}

- (void)stateChange:(MCRRoomSessionState)newState {
    self.status = newState;
    if ([self.delegate respondsToSelector:@selector(onRoomStateChanged:session:)]) {
        [self.delegate onRoomStateChanged:newState session:self];
    }
}
@end
