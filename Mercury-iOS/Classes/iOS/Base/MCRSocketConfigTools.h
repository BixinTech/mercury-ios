//
//  MCRMercurySetting+SocketConfig.h
//  Mercury
//
//  Created by 姜腾 on 2019/11/13.
//

#import "TunnelDefine.h"
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MCRSocketConfigTools : NSObject

+ (mercury::TunnelConfig)socketConfigFor:(NSDictionary *)diction;

@end

NS_ASSUME_NONNULL_END
