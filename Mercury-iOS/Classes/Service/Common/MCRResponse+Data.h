//
//  MCRResponse+Data.h
//  MercuryService
//
//  Created by Ju Liaoyuan on 2022/11/24.
//

#import "MCRResponse.h"

NS_ASSUME_NONNULL_BEGIN

@interface MCRResponse (Data)

+ (instancetype)responseFromData:(NSData *)data error:(NSError **)error;

@end

NS_ASSUME_NONNULL_END
