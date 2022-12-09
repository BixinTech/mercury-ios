//
//  MercuryService.m
//  MercuryService
//
//  Created by 姜腾 on 2019/11/20.
//

#import "MercuryService.h"
#import "MercuryHeader.h"
#import "MCRSettingService.h"
#import "AFNetworkReachabilityManager.h"
#import "MCRConstants.h"

@interface MercuryService ()<MCRMercuryDelegate>
{
    NSHashTable *_pushWeakDelegates;
    NSHashTable *_pushWeakMercuryServiceDelegates;
}
@property (nonatomic, strong) MCRMercury *mercury;
@property (nonatomic, assign) BOOL isSetuped;
@property (nonatomic, assign) BOOL isStarted;
@property (nonatomic, assign) BOOL isNetWorkFirstChange;
@property (nonatomic, strong) AFNetworkReachabilityManager *reachabilityManager;
@property (nonatomic, strong) NSMutableDictionary *eachMessageDiction;

@property (nonatomic, assign) NSInteger lastReportConnCode;
@property (nonatomic, assign) NSTimeInterval lastReportConnTime;

@property (atomic, assign) NSInteger errorReportInterval;

@property (atomic, assign) MercuryStatus connectionStatus;

@property (nonatomic, strong) NSMutableDictionary <NSString *, MCRPushDataHandler> *handleMap;

// protect handleMap read write opration
@property (nonatomic, strong) dispatch_queue_t readWriteQueue;
@end

@implementation MercuryService

+ (MercuryService *)shareInstance {
    
    static MercuryService * instance = nil;
    static  dispatch_once_t pred;
    dispatch_once(&pred, ^{
        instance = [[MercuryService alloc] init];
    });
    
    return instance;
}

- (id)init {
    if (self = [super init]) {
        _handleMap = [NSMutableDictionary dictionary];
        
        _readWriteQueue = dispatch_queue_create("com.MercuryService.DictionaryQueue", DISPATCH_QUEUE_SERIAL);
        
        _pushWeakDelegates = [[NSHashTable alloc] initWithOptions:NSPointerFunctionsWeakMemory capacity:0];
        
        _pushWeakMercuryServiceDelegates = [[NSHashTable alloc] initWithOptions:NSPointerFunctionsWeakMemory capacity:0];
        
        _mercury = [[MCRMercury alloc] initWithDelegate:self completedQueue:dispatch_get_main_queue()];
        _isStarted = NO;
        _isSetuped = NO;
        _isNetWorkFirstChange = YES;
        _eachMessageDiction = [[NSMutableDictionary alloc] init];
        _errorReportInterval = 180; //3 * 60
       // [self changeReachability];
    }
    return self;
}


- (void)registerNotification {
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillEnterForeground) name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidEnterBackground) name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(updateSetting:)
                                                 name:SETTINGUPDATE
                                               object:nil];
}

- (void)networkChange {
    if (self.isNetWorkFirstChange) {
        self.isNetWorkFirstChange = NO;
        return;
    }
    if (!self.isStarted) {
        return;
    }
    
    if (self.reachabilityManager.reachable) {
        MCRLOG(@"network change, reset connect");
        [self resetConnect];
   }
}

- (void)applicationDidEnterBackground {
    if (!self.isStarted) {
        return;
    }
    [self uploadAppState:NO];
    [self.mercury background:YES];
}

- (void)applicationWillEnterForeground {
    if (!self.isStarted) {
        return;
    }
    [self.mercury background:NO];
    NSTimeInterval t = [self.mercury timeFromLastMessage];
    if (t > 3.5 * 60 || ![self isConnected]) {
        MCRLOG(@"tunnel long time no send data, reset connect");
        [self resetConnect];
    }else {
        [self uploadAppState:YES];
    }
}

- (void)uploadAppState:(BOOL)isForeground {
    MCRMessage *m = [[MCRMessage alloc] init];
    m.command = MCRCommandReport;
    NSData *d = [NSJSONSerialization dataWithJSONObject:@{@"type":@"appstate",@"data":@{@"foreground":@(isForeground)}} options:0 error:NULL];
    if (!d) {
        return;
    }
    m.body = d;
    [self sendMessage:m];
}

- (void)resetConnect {
    if (!self.isSetuped) {
        return;
    }
    self.isStarted = YES;
    [self.mercury stopService];
    [self.mercury startService];
}

- (void)startService {
    if (self.isStarted) {
        return;
    }
    self.isStarted = YES;
    self.isSetuped = YES;
    
    [self setupService];
    [self.mercury startService];
}

- (void)stopService {
    if (!self.isStarted) {
        return;
    }
    self.isStarted = NO;
    [self.mercury stopService];
}

- (void)setupService {
    NSDictionary *d = [self.settingService settingDiction];
    [self updateErrorReportInterval:d];
    [self.mercury updateConfig:d];
    
    self.reachabilityManager = [AFNetworkReachabilityManager manager];
    __weak typeof(self) _weakSelf = self;
    [self.reachabilityManager setReachabilityStatusChangeBlock:^(AFNetworkReachabilityStatus status) {
        __strong typeof(_weakSelf) _strongSelf = _weakSelf;
        [_strongSelf networkChange];
    }];
    [self.reachabilityManager startMonitoring];
    
    [self.settingService start];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
       [self registerNotification];
    });
}

- (void)updateErrorReportInterval:(NSDictionary *)d {
    NSString *ei = [d objectForKey:@"ei"];
    if ([ei isKindOfClass:[NSString class]] || [ei isKindOfClass:[NSNumber class]]) {
        self.errorReportInterval = ei.intValue;
    }
}

- (void)updateSetting:(NSNotification *)notification {
    NSDictionary *dic = [notification object];
    if (!dic ||
        ![dic isKindOfClass:[NSDictionary class]]) {
        dic = [self.settingService settingDiction];
    }
    [self updateErrorReportInterval:dic];
    [self.mercury updateConfig:dic];
}

- (void)sendEachMessage {
    NSArray *messages = nil;
    @synchronized (self) {
        messages = [self.eachMessageDiction allValues];
    }
    for (MCRMessage *msg in messages) {
        [self sendMessage:msg];
    }
}

- (void)addEachMessage:(MCRMessage *)message type:(NSString *)type cancel:(BOOL)cancel {
    @synchronized (self) {
        if (cancel) {
            [self.eachMessageDiction removeObjectForKey:type];
        }else {
            [self.eachMessageDiction setObject:message forKey:type];
        }
    }
}


- (void)sendMessage:(MCRMessage *)msg {
    [self sendMessage:msg completion:nil];
}

- (void)sendMessage:(MCRMessage *)msg completion:(MCRRespCallback)completion {
    if (self.isStarted) {
        MCRLOG(@"mercury send message:%@",msg.description);
        [self.mercury sendMessage:msg completion:completion];
    }
}

- (BOOL)isConnected {
    return self.connectionStatus == MercuryStatusConnected;
}

- (BOOL)isConnecting {
    return self.connectionStatus == MercuryStatusConnecting;
}

- (BOOL)isDisconnect {
    return self.connectionStatus == MercuryStatusDisconnect;
}

- (void)addMCRPushDelegate:(id<MCRPushDelegate>)delegate {
    if (delegate) {
        @synchronized (self) {
            [_pushWeakDelegates addObject:delegate];
        }
    }
}

- (void)removeMCRPushDelegate:(id<MCRPushDelegate>)delegate {
    if (delegate && [_pushWeakDelegates containsObject:delegate]) {
        @synchronized (self) {
            [_pushWeakDelegates removeObject:delegate];
        }
    }
}

- (void)addMercuryServiceDelegate:(id<MercuryServiceDelegate>)delegate {
    if (delegate) {
        @synchronized (self) {
            [_pushWeakMercuryServiceDelegates addObject:delegate];
        }
    }
}

- (void)removeMercuryServiceDelegate:(id<MercuryServiceDelegate>)delegate {
    if (delegate && [_pushWeakMercuryServiceDelegates containsObject:delegate]) {
        @synchronized (self) {
            [_pushWeakMercuryServiceDelegates removeObject:delegate];
        }
    }
}

#pragma mark - mercury delegate

- (void)onReadPackage:(MCRMessage *)package mercury:(MCRMercury *)mercury error:(nonnull NSError *)error {
    MCRHeader *batchHeader = [self batchHeaderForHeaders:package.headers];
    if (batchHeader) {
        NSArray<NSData *> *bodyArr = [self batchBodyForHeader:batchHeader body:package.body];
        for (NSData *body in bodyArr) {
            MCRMessage *message = [MCRMessage new];
            message.command = package.command;
            message.body = body;
            message.headers = package.headers;
            [self handleSimplePackage:message mercury:mercury];
        }
    } else {
        [self handleSimplePackage:package mercury:mercury];
    }
}

- (void)handleSimplePackage:(MCRMessage *)package mercury:(MCRMercury *)mercury {
    switch (package.command) {
        case MCRCommandResetConnection:{
            [self resetConnect];
        }
            break;
        default:{
            [self.delegate onHandlePackage:package];
        }
            break;
    }
//    MCRLOG(@"MS:mercury received message:%@",package.description);
}

- (NSInteger)tunnelType {
    if ([self.delegate respondsToSelector:@selector(tunnelType)]) {
        return [self.delegate tunnelType];
    }
    
    return 0;
}

- (void)onTunnelStatus:(MercuryStatus)status {
    self.connectionStatus = status;
    
    if (status == MercuryStatusConnected) {
        [self sendEachMessage];
    }
    
    
    if ([self.delegate respondsToSelector:@selector(onTunnelStatus:)]) {
        [self.delegate onTunnelStatus:status];
    }
    
    @synchronized (self) {
        for (id<MercuryServiceDelegate>serviceDelegate in _pushWeakMercuryServiceDelegates.copy) {
             if ([serviceDelegate respondsToSelector:@selector(onTunnelStatus:)]) {
                 [serviceDelegate onTunnelStatus:status];
             }
        }
    }
}

- (NSDictionary *)objectWithData:(NSData *)jsonData {
    if (!jsonData || jsonData.length == 0) {
        return nil;
    }
    NSError *error;
    id object = [NSJSONSerialization JSONObjectWithData:jsonData options:0 error:&error];
    if (error) {
        return nil;
    }
    if ([object isKindOfClass:[NSDictionary class]]) {
        return object;
    }else {
        return nil;
    }
}

- (void)pvWithCommandCode:(NSInteger)commandCode code:(NSInteger)code responseTime:(NSInteger)time uploadSample:(float)uploadSample {
    NSInteger aCode = code;
    if (aCode == 0) {
        aCode = 200;
    }
    NSString *command = @"mercury/unknown";
    switch (commandCode) {
        case 0:
            command = @"mercury/dns_resolve";
            break;
        case 1:
            command = @"mercury/connect";
            break;
        case 2:
            command = @"mercury/handshake";
            break;
        default:
            break;
    }
    
    BOOL b = YES;
    if ([command isEqualToString:@"mercury/connect"]) {
        b = [self shouldUpload:command code:aCode];
        if (b &&
            aCode != 200 &&
            !self.reachabilityManager.reachable) {
            command = @"mercury/connect_not_network";
        }
    }
    
    if (b) {
        /// upload data
    }
}

- (BOOL)shouldUpload:(NSString *)command code:(NSInteger)code {
    
    BOOL b = YES;
    if (code != 200) {
        NSTimeInterval ctime= [[NSDate new] timeIntervalSince1970] - self.lastReportConnTime;
        if (code == self.lastReportConnCode &&
            ctime < self.errorReportInterval) {
            b = NO;
        }
    }
    if (b) {
        self.lastReportConnCode = code;
        self.lastReportConnTime = [[NSDate new] timeIntervalSince1970];
    }
    
    return b;
}

- (MCRSettingService *)settingService {
    return [MCRSettingService shareInstance];
}

- (MCRHeader *)batchHeaderForHeaders:(NSArray<MCRHeader *> *)headers {
    for (MCRHeader *h in headers) {
        if (h.type == MCRHeaderTypeBatch) {
            return h;
        }
    }
    return nil;
}

- (NSArray<NSData *> *)batchBodyForHeader:(MCRHeader *)header body:(NSData *)body {
    NSInteger bodyLength = [[[NSString alloc] initWithData:header.data encoding:NSUTF8StringEncoding] integerValue];
    Byte bodyLengthBytes[4];
    NSInteger offset = 0;
    NSMutableArray *dataArr = [NSMutableArray array];
    for (int i = 0; i < bodyLength; i++) {
        if (offset + 4 < body.length) {
            [body getBytes:bodyLengthBytes range:NSMakeRange(offset, 4)];
        }
        int bodyLength = (int)((bodyLengthBytes[3] & 0xff) |
                            ((bodyLengthBytes[2] << 8) & 0xff00) |
                            ((bodyLengthBytes[1] << 16) & 0xff0000) |
                            ((bodyLengthBytes[0] << 24) & 0xff000000));
        if (offset + 4 + bodyLength < body.length) {
            NSData *subBodyData = [body subdataWithRange:NSMakeRange((offset + 4), bodyLength)];
            [dataArr addObject:subBodyData];
        }
        offset += (4 + bodyLength);
    }
    return dataArr;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}
@end
