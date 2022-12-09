//
//  MCRMercurySetting.m
//  Mercury
//
//  Created by 姜腾 on 2019/11/13.
//

#import "MCRSettingService.h"
#import <MCRLog.h>

#define kMercurySettingDataKey @"kMercurySettingDataKey"

@interface MCRSettingService ()
@property (nonatomic, strong) NSUserDefaults *settingUserDefault;
@property (nonatomic, assign) BOOL isStarted;
@end

@implementation MCRSettingService

+ (MCRSettingService *)shareInstance {
    
    static MCRSettingService * instance = nil;
    static  dispatch_once_t pred;
    dispatch_once(&pred, ^{
        instance = [[MCRSettingService alloc] init];
    });
    
    return instance;
}

- (void)start {
    if (self.isStarted) {
        return;
    }
    self.isStarted = YES;
}


- (NSDictionary *)settingDiction {
    NSDictionary *d = [self.settingUserDefault objectForKey:kMercurySettingDataKey];
    if (![d isKindOfClass:[NSDictionary class]]) {
        d = [self defaultParams];
    }
    MCRLOG(@"get config data:%@",d);
    return d;
}

- (NSDictionary *)defaultParams {
    return @{
        @"ct":@"8",
        @"c":@"false",
        @"pt":@"25",
        @"ei":@"180",
        @"cbc":@"false",
        @"pi":@"500",
        @"bpi":@"500",
        @"ht":@"25"
    };
}

- (NSUserDefaults *)settingUserDefault {
    if (_settingUserDefault == nil) {
        _settingUserDefault = [[NSUserDefaults alloc] initWithSuiteName:@"MercurySetting"];
    }
    return _settingUserDefault;
}

@end
