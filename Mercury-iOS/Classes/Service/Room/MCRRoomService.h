//
//  MCRoomService.h
//  MercuryService
//
//  Created by 姜腾 on 2020/4/14.
//

#import <Foundation/Foundation.h>
#import "MercuryService.h"
#import "MCRRoomModel.h"
#import "MCRRoomSession.h"

NS_ASSUME_NONNULL_BEGIN

@interface MCRRoomService : NSObject
@property (nonatomic, strong) MercuryService *tunnelServer;

+ (MCRRoomService *)shareInstance;

- (void)enterChatroom:(MCRRoomSession *)session;

- (void)exitChatroom:(MCRRoomSession *)session;

/// 发送消息
/// @param model 消息
- (void)sendMessage:(MCRRoomModel *)model;

/// 发送消息
/// @param model 消息
/// @param completion resp 回调
- (void)sendMessage:(MCRRoomModel *)model completion:(MCRRespCallback _Nullable)completion;

- (void)deleteChatroom;

@end

NS_ASSUME_NONNULL_END
