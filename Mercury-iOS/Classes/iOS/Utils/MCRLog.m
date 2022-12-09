//
//  MCRLog.m
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import <sys/time.h>
#import "MCRLog.h"

@interface MCRLog()
@property (nonatomic, copy) MCRMCRLogBlock logBlock;
@end

@implementation MCRLog

+ (instancetype)sharedInstance {
    static MCRLog *instance = nil;
    static dispatch_once_t once_log;
    dispatch_once(&once_log, ^{
        instance = [[MCRLog alloc] init];
    });
    return instance;
}

- (void)setupLogBlock:(MCRMCRLogBlock)block {
    self.logBlock = block;
    
}

- (void)log:(NSString *)log {
    if (self.logBlock) {
        self.logBlock(log);
    }
}
@end

void __MCRLog(NSString *file, NSInteger line, NSString * content) {
    NSString *str = [[NSString alloc] initWithFormat:@"Mercury>> %@", content];
    [[MCRLog sharedInstance] log:str];
}




