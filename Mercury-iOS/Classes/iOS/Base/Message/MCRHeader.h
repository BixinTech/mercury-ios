//
//  MCRHeader.h
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import <Foundation/Foundation.h>
#import "MCRConstants.h"

NS_ASSUME_NONNULL_BEGIN

@interface MCRHeader : NSObject
@property(nonatomic, assign) MCRHeaderType type;
@property(nonatomic, strong) NSData *data;

- (NSString *)description;
@end

NS_ASSUME_NONNULL_END
