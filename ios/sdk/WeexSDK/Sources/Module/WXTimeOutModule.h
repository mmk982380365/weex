//
//  WXTimeOutManager.h
//  WeexSDK
//
//  Created by openthread on 8/12/20.
//

#include <AliQuickJS/quickjs.h>

void WXQJSSetTimeOut(JSContext *ctx, JSValue func, JSValue this_val, int32_t interval_ms);
