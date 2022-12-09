//
//  MCRResponse+Data.m
//  MercuryService
//
//  Created by Ju Liaoyuan on 2022/11/24.
//

#import "MCRResponse+Data.h"
#import "MCRConstants.h"

@implementation MCRResponse (Data)

+ (instancetype)responseFromData:(NSData *)data error:(NSError *__autoreleasing  _Nullable *)error {
    NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:data
                                                         options:NSJSONReadingMutableContainers
                                                           error:error];
    MCRResponse *resp = nil;
    if (!*error) {
        /// 序列化成功
        NSInteger code = [dict[@"c"] intValue];
        resp = [MCRResponse new];
        resp.code = code;
        if (code != MCRErrorCodeSuccess) {
            /// resp 失败
            *error = [NSError errorWithDomain:MCRErrorDomain
                                         code:code
                                     userInfo:@{NSLocalizedDescriptionKey:[NSString stringWithFormat:@"%@", dict[@"d"]]}];
        } else {
            resp.data = dict[@"d"];
        }
    }
    return resp;
}

@end
