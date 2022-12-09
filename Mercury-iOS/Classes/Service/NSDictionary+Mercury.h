//
//  NSDictionary+Mercury.h
//  MercuryService
//
//  Created by 姜腾 on 2020/2/25.
//


#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSDictionary (Mercury)
+ (NSDictionary *)mercuryObjectWithData:(NSData *)jsonData;


- (BOOL)mercuryBoolForKey:(NSString *)key;
- (NSInteger)mercuryIntegerForKey:(NSString *)key;
- (NSDictionary *)mercuryDictionForKey:(NSString *)key;
- (NSString *)mercuryStringForKey:(NSString *)key;
- (NSArray *)mercuryArrayForKey:(NSString *)key;

@end

NS_ASSUME_NONNULL_END
