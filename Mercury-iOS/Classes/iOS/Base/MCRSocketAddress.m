//
//  MCRSocketAddress.m
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import "MCRSocketAddress.h"

@implementation MCRSocketAddress

+ (instancetype)addressWithHost:(NSString *)host andPort:(UInt32)port {
    MCRSocketAddress *obj = [[MCRSocketAddress alloc] init];
    obj.host = host;
    obj.port = port;
    return obj;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"%@:%d", self.host, (unsigned int)self.port];
}
@end
