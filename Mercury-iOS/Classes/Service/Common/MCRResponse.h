//
//  MCRResponse.h
//  MercuryService
//
//  Created by Ju Liaoyuan on 2022/11/2.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MCRResponse;

typedef void(^MCRRespCallback)(MCRResponse *_Nullable resp, NSError *_Nullable error);

@interface MCRResponse : NSObject

/// response's code
@property (nonatomic, assign) NSInteger code;

/// response's data
@property (nonatomic, strong) NSDictionary *data;

@end

NS_ASSUME_NONNULL_END
