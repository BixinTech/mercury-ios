//
//  MCRRoomSession.h
//  MercuryService
//
//  Created by 姜腾 on 2020/4/16.
//

#import <Foundation/Foundation.h>
#import "MCRRoomModel.h"
#import "MercuryService.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger,MCRRoomSessionState){
    MCRRoomSessionStateLeaveRoom        = 0,
    MCRRoomSessionStateEnteringRoom     = 1,
    MCRRoomSessionStateInRoom           = 2
};

@class MCRRoomSession;
@protocol MCRRoomSessionDelegate <NSObject>
@optional

/// 收到IM普通消息
/// @param model 消息内容
- (void)onRoomMessage:(MCRRoomModel *)model session:(MCRRoomSession*)session;


/// 收到IM指令消息
/// @param model 消息内容
- (void)onNotificationMessage:(MCRRoomModel *)model session:(MCRRoomSession*)session;

/// 连接状态
/// @param state 状态
- (void)onConnectionStateChanged:(MercuryStatus)state session:(MCRRoomSession*)session;

/// 房间状态
/// @param state 状态
- (void)onRoomStateChanged:(MCRRoomSessionState)state session:(MCRRoomSession*)session;

/// 被动退出房间
/// @param result 退出原因
- (void)onKickRoom:(NSString *)roomid result:(NSData *)result session:(MCRRoomSession*)session;


/// 加入房间结果
/// @param roomid 房间ID
/// @param result nil，表示成功。
- (void)onEnterRoom:(NSString *)roomid result:(nullable NSError *)result session:(MCRRoomSession*)session;

@end


@interface MCRRoomSession : NSObject
@property (atomic, assign) MCRRoomSessionState status;

@property (nonatomic, weak) id<MCRRoomSessionDelegate> delegate;

/// roomId
@property (nonatomic, copy) NSString *roomId;

/// user id
@property (nonatomic, strong) NSString *uid;

/// 是否是游客
@property (nonatomic, assign, getter=isGuest) BOOL guest;

/// 进入房间时的扩展字段
@property (nonatomic, nullable,strong) NSString *enterExt;

/// 进入房间
- (void)enterRoom;

/// 发送消息
/// @param data 消息
- (void)sendMessage:(NSData *)data;

/// 发送ack消息
- (void)sendAckMessage:(NSData *)data;

/// 发送消息
/// @param data 消息内容
/// @param completion 响应体
- (void)sendMessage:(NSData *)data completion:(_Nullable MCRRespCallback)completion;

/// 离开房间
- (void)exitRoom;

///// 销毁会话！！！！！！！
- (void)destoryRoom;


- (void)stateChange:(MCRRoomSessionState)newState;
@end

NS_ASSUME_NONNULL_END
