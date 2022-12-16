# Mercury-iOS

![](https://img.shields.io/badge/Pod-v1.0.0-brightgreen)

Mercury 基于 C++ boost 的 asio，使用私有协议，构建了一套自建长链。包含了通道建连，消息收发（支持 twoway），ack 机制，房间管理，错误处理等核心功能。在比心，Mercury 经过了两年多的线上业务验证，现在已经在比心全系列 App 中使用。

# Get Started

## 1. 安装

Mercury-iOS 支持源码及 CocoaPods 集成。

### 1.1 源码

1. 下载本仓库，将 Mercury-iOS 文件夹拖入到工程中。
2. 由于依赖了 AFNetworking 的网络变化检测，需要确保工程中以及引入了 AFNetworking

### 1.2 CocoaPods

在 Podfile 中，添加依赖：

```
pod 'Mercury-iOS'
```

由于依赖了 AFNetworking 的网络变化检测，需要同时在 Podfile 中添加 AFNetworking 的依赖


## 2. 使用

### 2.1 配置服务地址和端口号

在合适的时机（建议 App 启动时），配置 Mercury 的后端服务地址和端口号

```
#import "MCRConfigurator.h"

[[MCRConfigurator configurator] setupAddress:^MCRSocketAddress * _Nullable{
    return [MCRSocketAddress addressWithHost:@"xxx.xxx.xxx.xxx" andPort:8080];
}];
```

### 2.2 初始化 Session

```
_roomSession = [MCRRoomSession new];
_roomSession.delegate = self;
```

### 2.3 进入房间

```
// 配置 roomId
self.roomSession.roomid = @"12345";
// 配置 uid
self.roomSession.uid = @"12345";

// 进入房间
[self.roomSession enterRoom];
```

如果是第一次进入房间，Mercury 会自动建立一条通道，并保持通道复用，再次进入房间不会重复创建通道。

调用 `enterRoom` 后，可以在 `- onEnterRoom:result:session:` 中获取进入房间结果。如果 `result` 是 `nil`，代表成功，否则都是
失败，具体失败原因通过 `result` 来获取。

### 2.4 发送消息

Mercruy 支持发送文本或者二进制消息。通过调用
```
[self.roomSession sendMessage:data];
```
发送消息。

或者通过
```
[self.roomSession sendMessage:data:completion ^(MCRResponse * _Nullable resp, NSError * _Nullable error) {
    if (!error) {
        // send success
    } else {
        // send error
    }
}];
```
来发送消息，并接受发送结果（twoway）

### 2.5 接收消息

Mercury 收到消息后，会触发 `- onRoomMessage:session:` 回调，可通过实现此回调来接收消息。


### 2.6 连接状态

当通道连接状态发生改变时，会触发 `- onConnectionStateChanged:session:` 回调，可根据 MercuryStatus 值来做对应的处理。如重连。

### 2.7 房间状态

当房间状态发生改变时，会触发 `- onRoomStateChanged:session:` 回调，可根据 MCRRoomSessionState 值来做对应的处理。

### 2.8 离开房间

调用 `- exitRoom` 离开房间，并且同时调用 `- destroyRoom`销毁本次回话。

# 最佳实践

在比心开源的[Sona](https://github.com/BixinTech/sona-ios)中，对 Mercury 做了二次封装，对上提供了 IM 能力，可以参考 Sona 中 [SNMCRConnection](https://github.com/BixinTech/sona-ios/tree/main/Sona/Classes/SonaConn/Mercury)的实现。

# License

Mercury is Apache 2.0 licensed, as found in the [LICENSE](https://github.com/BixinTech/mercury-ios/blob/main/LICENSE) file.
