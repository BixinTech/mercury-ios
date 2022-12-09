//
//  MCRHeader.m
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import "MCRHeader.h"

@implementation MCRHeader

- (NSString *)description {
    return [NSString stringWithFormat:@"type:%ld data:%@",self.type, self.data];
}
@end
