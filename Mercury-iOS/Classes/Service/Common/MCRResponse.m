//
//  MCRResponse.m
//  MercuryService
//
//  Created by Ju Liaoyuan on 2022/11/2.
//

#import "MCRResponse.h"

@implementation MCRResponse

- (NSString *)description {
    return [NSString stringWithFormat:@"code: %@, data: %@", @(self.code), self.data];
}

@end
