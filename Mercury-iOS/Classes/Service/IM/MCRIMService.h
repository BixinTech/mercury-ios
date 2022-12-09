//
//  MCRIMService.h
//  MercuryService
//
//  Created by 姜腾 on 2020/4/16.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MCRIMServiceDelegate <NSObject>
@optional


/// 收到群组消息
/// @param messageData 消息体
/// @param groupid groupid
- (void)onGroupMessage:(NSData *)messageData groupid:(NSString *)groupid;


/// 收到离线消息
/// @param requestSyncData 消息体
/// @param code 暂定 1成功 0失败 
- (void)syncSessionInfoCode:(NSInteger)code requestSyncData:(NSData *)requestSyncData;

/// 收到实时消息
/// @param requestSyncData 消息体
/// @param code 暂定 1成功 0失败
- (void)handleSyncRealTimeIMMessage:(NSInteger)code requestSyncData:(NSData *)requestSyncData;

/// 接收ACK消息
/// @param requestSyncData 消息体
/// @param code 暂定 1成功 0失败
- (void)handleSyncRealACKMessage:(NSInteger)code requestSyncData:(NSData *)requestSyncData;

/// 接收已读回执消息
/// @param code 暂定 1成功 0失败
/// @param receiptData 数据
- (void)handleReceiptMessage:(NSInteger)code receiptData:(NSData *)receiptData;

@end

@class MCRMessage;
@interface MCRIMService : NSObject

- (void)sendGroupMessage:(NSData *)messageData groupid:(NSString *)groupid;

- (void)addDelegate:(id<MCRIMServiceDelegate>)delegate;

- (void)removeDelegate:(id<MCRIMServiceDelegate>)delegate;

- (void)receiveIMMessage:(MCRMessage *)package;

- (BOOL)canHandleCommand:(NSInteger)command;
@end

NS_ASSUME_NONNULL_END
