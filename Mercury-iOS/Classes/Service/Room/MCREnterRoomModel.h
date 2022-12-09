//
//  MCEnterRoomModel.h
//  MercuryService
//
//  Created by 姜腾 on 2020/4/14.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MCREnterRoomModel : NSObject
@property (nonatomic, strong) NSString *uid;
@property (nonatomic, copy) NSString *roomId;
@property (nonatomic, strong) NSString *ext;
@end

NS_ASSUME_NONNULL_END
