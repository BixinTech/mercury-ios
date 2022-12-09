//
//  MCRMercurySetting+SocketConfig.m
//  Mercury
//
//  Created by 姜腾 on 2019/11/13.
//

#import "MCRSocketConfigTools.h"


@implementation MCRSocketConfigTools
+ (mercury::TunnelConfig)socketConfigFor:(NSDictionary *)diction {
    mercury::TunnelConfig config;
    config.closeMercury = [self boolForKey:@"c" diction:diction deflaut:config.closeMercury];
    
    config.pingTimeout = (uint32_t)[self intForKey:@"pt" diction:diction deflaut:config.pingTimeout];
    config.shakehandTimeout = (uint32_t)[self intForKey:@"ht" diction:diction deflaut:config.shakehandTimeout];
    config.connectTimeout = (uint32_t)[self intForKey:@"ct" diction:diction deflaut:config.connectTimeout];
    config.pingInterval = (uint32_t)[self intForKey:@"pi" diction:diction deflaut:config.pingInterval];
    config.pingIntervalBackground = (uint32_t)[self intForKey:@"bpi" diction:diction deflaut:config.pingIntervalBackground];
    
    return config;
}

+ (BOOL)boolForKey:(NSString *)key diction:(NSDictionary *)d deflaut:(BOOL)b {
    BOOL result = b;
    if (!key.length) {
        return result;
    }
    NSNumber *bValue = [d objectForKey:key];
    if ([bValue isKindOfClass:[NSString class]] || [bValue isKindOfClass:[NSNumber class]]) {
        result = [bValue boolValue];
    }
    return result;
}

+ (NSInteger)intForKey:(NSString *)key diction:(NSDictionary *)d deflaut:(NSInteger)i {
    
    NSInteger result = i;
    if (!key.length) {
        return result;
    }
    NSNumber *bValue = [d objectForKey:key];
    if ([bValue isKindOfClass:[NSString class]] || [bValue isKindOfClass:[NSNumber class]]) {
        result = [bValue integerValue];
        if (result <= 0) {
            result = i;
        }
    }
    return result;
}

@end
