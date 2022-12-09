//
//  MCRMercurySetting.h
//  Mercury
//
//  Created by 姜腾 on 2019/11/13.
//

#import <Foundation/Foundation.h>

#define SETTINGUPDATE @"kSETTINGUPDATE"

NS_ASSUME_NONNULL_BEGIN

@interface MCRSettingService : NSObject

+ (MCRSettingService *)shareInstance;

- (void)start;
- (NSDictionary *)settingDiction;
@end

NS_ASSUME_NONNULL_END
