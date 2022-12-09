//
//  MCRoomModel.h
//  MercuryService
//
//  Created by 姜腾 on 2020/4/14.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MCRRoomModel : NSObject

/// 发送数据。发送的数据长度不允许超过50KB, 推荐不超过4KB，发送周期建议控制在50ms以上
@property (nonatomic, strong) NSData *data;

/// 房间ID
@property (nonatomic, copy) NSString *roomId;

/// 是否ack消息
@property (nonatomic, assign) BOOL isAckMsg;

/// 透传消息
@property (nonatomic, strong) NSDictionary *ext;

@end

NS_ASSUME_NONNULL_END
