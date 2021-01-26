//
//  WXTimeOutManager.m
//  WeexSDK
//
//  Created by openthread on 8/12/20.
//

#import <Foundation/Foundation.h>
#import "WXTimeOutModule.h"

@interface WXTimeOutModule : NSObject

+ (instancetype)sharedInstance;

+ (void)delayTrigger:(void(^)(void))func interval:(double)interval;

@end

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

void WXQJSSetTimeOut(JSContext *ctx, JSValue func, JSValue this_val, int32_t interval_ms)
{
    [WXTimeOutModule delayTrigger:^{
        if (JS_IsFunction(ctx, func))
        {
            JS_Call(ctx, func, this_val, 0, NULL);
        }
        JS_FreeValue(ctx, func);
    } interval:interval_ms / 1000.f];
}
