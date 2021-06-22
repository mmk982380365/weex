/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
//
// Created by Huiying Jiang on 2021/6/20.
//

#include <map>
#include <mutex>

#ifndef WEEX_PROJECT_WEEX_TIMER_MANAGER_QJS_H
#define WEEX_PROJECT_WEEX_TIMER_MANAGER_QJS_H
#include "base/log_defines.h"
extern "C" {
#if OS_ANDROID
#include "include/qjs/quickjs.h"
#elif OS_IOS
#import <AliQuickJS/quickjs.h>
#endif
}

#if OS_ANDROID
#include <wson.h>
#include "wson_parser.h"
#include "android/utils/wson_qjs.h"
#endif
class WeexTimerManagerQJS{
private:
    std::map<uint32_t, JSValue> timer_function_;
    std::mutex timer_functions_op_mutex_;
    uint32_t timer_function_id_;

public:
    explicit WeexTimerManagerQJS() {
        timer_function_id_=0;
    }
    uint32_t gen_timer_function_id() {
        return ++timer_function_id_;
    }

    JSValue get_timer_function(uint32_t function_id) {
        std::unique_lock<std::mutex> scoped_lock(timer_functions_op_mutex_);
        auto iter = timer_function_.find(function_id);
        if (iter == timer_function_.end()) {
            return JS_UNDEFINED;
        }
        return iter->second;
    }

    JSValue remove_timer(uint32_t function_id) {
        std::unique_lock<std::mutex> scoped_lock(timer_functions_op_mutex_);
        auto iter = timer_function_.find(function_id);
        if (iter == timer_function_.end()) {
            LOGE("timer do not exist!");
            return JS_UNDEFINED;
        }
        JSValue function = iter->second;
        timer_function_.erase(function_id);
        return function;
    }

    inline void add_timer(uint32_t function_id, JSValue func) {
        std::unique_lock<std::mutex> scoped_lock(timer_functions_op_mutex_);
        timer_function_[function_id] = func;
    }
    void clear_timer() {
        std::unique_lock<std::mutex> scoped_lock(timer_functions_op_mutex_);
        timer_function_.clear();
    }
};
#endif //WEEX_PROJECT_WEEX_TIMER_MANAGER_QJS_H
