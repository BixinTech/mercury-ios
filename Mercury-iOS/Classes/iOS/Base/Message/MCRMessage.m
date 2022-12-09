//
//  MCRMessage.m
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import "MCRMessage.h"
#import "MCRConstants.h"

@implementation MCRMessage

+ (instancetype)twoWayRequest {
    MCRMessage *instance = [MCRMessage new];
    instance.type = MCRMessageTypeReq;
    instance.isTwoWay = true;
    return instance;
}

- (NSString *)description {
    NSMutableString *hstring = [[NSMutableString alloc] init];
    [hstring appendString:@"("];
    for (MCRHeader *h in self.headers) {
        [hstring appendString:[NSString stringWithFormat:@"%@",h]];
    }
    [hstring appendString:@")"];
    return [NSString stringWithFormat:@"c:%ld headers:%@ body:%@",self.command,hstring,!self.body?@"":[[NSString alloc] initWithData:self.body encoding:NSUTF8StringEncoding]];
}
@end
