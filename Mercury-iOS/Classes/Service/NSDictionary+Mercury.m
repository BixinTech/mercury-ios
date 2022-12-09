//
//  NSDictionary+Mercury.m
//  MercuryService
//
//  Created by 姜腾 on 2020/2/25.
//

#import "NSDictionary+Mercury.h"


@implementation NSDictionary (Mercury)

+ (NSDictionary *)mercuryObjectWithData:(NSData *)jsonData {
    if (!jsonData || jsonData.length == 0) {
        return nil;
    }
    NSError *error;
    id object = [NSJSONSerialization JSONObjectWithData:jsonData options:0 error:&error];
    if (error) {
        return nil;
    }
    if ([object isKindOfClass:[NSDictionary class]]) {
        return object;
    }else {
        return nil;
    }
}


- (BOOL)mercuryBoolForKey:(NSString *)key {
    if (!key.length) {
        return NO;
    }
    BOOL b = NO;
    NSString *value = [self objectForKey:key];
    if ([value isKindOfClass:[NSString class]] || [value isKindOfClass:[NSNumber class]]) {
        b = [value boolValue];
    }
    return b;
}

- (NSInteger)mercuryIntegerForKey:(NSString *)key {
    NSInteger i = 0;
    if (!key.length) {
        return i;
    }
    NSString *s = [self objectForKey:key];
    if ([s isKindOfClass:[NSString class]] || [s isKindOfClass:[NSNumber class]]) {
        i = [s integerValue];
    }
    return i;
}

- (NSDictionary *)mercuryDictionForKey:(NSString *)key {
    NSDictionary *result = @{};
    if (!key.length) {
        return result;
    }
    NSDictionary *s = [self objectForKey:key];
    if ([s isKindOfClass:[NSDictionary class]]) {
        result = s;
    }
    return result;
}

- (NSString *)mercuryStringForKey:(NSString *)key {
    NSString *result = @"";
    if (!key.length) {
        return result;
    }
    NSString *s = [self objectForKey:key];
    if ([s isKindOfClass:[NSString class]]) {
        result = s;
    }
    return result;
}

- (NSArray *)mercuryArrayForKey:(NSString *)key {
    NSArray *result = @[];
    if (!key.length) {
        return result;
    }
    NSArray *s = [self objectForKey:key];
    if ([s isKindOfClass:[NSArray class]]) {
        result = s;
    }
    return result;
}


@end
