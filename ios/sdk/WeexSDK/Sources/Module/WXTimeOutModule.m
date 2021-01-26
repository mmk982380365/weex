//
//  WXTimeOutManager.m
//  WeexSDK
//
//  Created by openthread on 8/12/20.
//

#import "WXTimeOutModule.h"

@implementation WXTimeOutModule

+ (instancetype)sharedInstance
{
    static id instance = nil;
    return instance;
}

+ (void)delayTrigger:(void(^)(void))func interval:(double)interval
{
    [self performSelector:@selector(triggerTimeout:) withObject:func afterDelay:interval];
}

+ (void)triggerTimeout:(void(^)(void))block
{
    if (block)
    {
        block();
    }
}

@end

