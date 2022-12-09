//
//  MCRLog.h
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^MCRMCRLogBlock)(NSString * log);

@interface MCRLog : NSObject

+ (instancetype)sharedInstance;

- (void)setupLogBlock:(MCRMCRLogBlock)block;

@end

NS_ASSUME_NONNULL_END

#ifndef __FILE_NAME__
#define __FILE_NAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define MCRLOG(format, ...) __MCRLog(@__FILE_NAME__, __LINE__, [NSString stringWithFormat:format, ## __VA_ARGS__]);

#ifdef __cplusplus
extern "C"
{
#endif
void __MCRLog(NSString * _Nullable file, NSInteger line, NSString * _Nullable content);
#ifdef __cplusplus
}
#endif



