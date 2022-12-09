//
//  MCRConfigurator.m
//  Mercury
//
//  Created by 姜腾 on 2019/11/7.
//

#import "MCRConfigurator.h"
#import <UIKit/UIKit.h>
#include <sys/sysctl.h>

@implementation MCRConfigurator {
    MCRConfigStringBlock _unionIdBlock;
    MCRConfigStringBlock _userIdBlock;
    MCRConfigStringBlock _bundleIdBlock;
    MCRAddressConfigBlock _addressBlock;
    NSTimeInterval _timeoutInterval;
}
+ (MCRConfigurator *)configurator {
    
    static MCRConfigurator * instance = nil;
    static  dispatch_once_t pred;
    dispatch_once(&pred, ^{
        instance = [[MCRConfigurator alloc] _init];
    });
    
    return instance;
}

- (instancetype)_init {
    self = [super init];
    if (self) {
        _timeoutInterval = 300;
    }
    return self;
}

#pragma mark - Getter

- (NSString *)unionId {
    if (_unionIdBlock) {
        return _unionIdBlock();
    }
    return nil;
}

- (NSString *)userId {
    if (_userIdBlock) {
        return _userIdBlock();
    }
    return nil;
}

- (NSString *)bundleId {
    if (_bundleIdBlock) {
        return _bundleIdBlock();
    }
    return nil;
}

- (MCRSocketAddress *)address {
    if (_addressBlock) {
        return _addressBlock();
    }
    return nil;
}

- (NSTimeInterval)sendMsgTimeoutInterval {
    return _timeoutInterval;
}

#pragma mark - Setter

- (void)setupUserId:(MCRConfigStringBlock)block {
    _userIdBlock = block;
}
- (void)setupUnionid:(MCRConfigStringBlock)block {
    _unionIdBlock = block;
}
- (void)setupBundleId:(MCRConfigStringBlock)block {
    _bundleIdBlock = block;
}

- (void)setupAddress:(MCRAddressConfigBlock)block {
    _addressBlock = block;
}

- (void)setupSendMsgTimeoutInterval:(NSTimeInterval)interval {
    _timeoutInterval = interval;
}

+ (NSString *)systemVersion {
    return [[UIDevice currentDevice] systemVersion];
}

+ (NSString *)appVersion {
    NSString *v = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    return v?:@"";
}

+ (NSString *)platformString {
    static NSString *platform = nil;
    if (!platform) {
        size_t size;
        sysctlbyname("hw.machine", NULL, &size, NULL, 0);
        char *machine = malloc(size);
        sysctlbyname("hw.machine", machine, &size, NULL, 0);
        platform = [NSString stringWithCString:machine encoding:NSUTF8StringEncoding];
        free(machine);
    }
    return platform;
}

@end
