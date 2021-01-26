//
//  WXTimeOutManager.h
//  WeexSDK
//
//  Created by openthread on 8/12/20.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface WXTimeOutModule : NSObject

+ (instancetype)sharedInstance;

+ (void)delayTrigger:(void(^)(void))func interval:(double)interval;

@end


NS_ASSUME_NONNULL_END
