//
//  MCRConstants.h
//  Pods
//
//  Created by Ju Liaoyuan on 2022/10/31.
//

#ifndef MCRConstants_h
#define MCRConstants_h

typedef NS_ENUM(NSInteger, MCRCommand) {
    /// 重置连接
    MCRCommandResetConnection   = -1,
    /// 客户端上报
    MCRCommandReport            = 2,
    /// 进入房间
    MCRCommandEnterRoom         = 10,
    /// 离开房间
    MCRCommandLeaveRoom         = 11,
    /// 发送房间消息
    MCRCommandSendRoomMessage   = 12,
    /// 发送房间指令
    MCRCommandSendRoomCmd       = 13,
    /// 发送群组消息
    MCRCommandSendGroupMessage  = 14,
};

typedef NS_ENUM(NSInteger, MCRSubCommand) {
    /// 被踢出
    MCRSubCommandBeKicked   = 1,
    /// 房间关闭
    MCRSubCommandCloseRoom  = 2,
};

typedef NS_ENUM(NSInteger, MCRHeaderType) {
    /// 压缩
    MCRHeaderTypeZip    = 1,
    /// 房间
    MCRHeaderTypeRoom   = 2,
    /// 批量
    MCRHeaderTypeBatch  = 3,
};

typedef NS_ENUM(NSInteger, MCRMessageType) {
    /// 返回
    MCRMessageTypeResp  = 0,
    /// 请求
    MCRMessageTypeReq   = 1,
};

typedef NS_ENUM(NSInteger, MCRErrorCode) {
    /// 无错误
    MCRErrorCodeSuccess = 0,
    /// 接收 Resp 超时
    MCRErrorCodeTimeout = -10,
};

#define MCRErrorDomain @"Mercury-iOS"

#endif /* MCRConstants_h */


