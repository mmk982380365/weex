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
// Created by 董亚运 on 2020/8/8.
//

#ifndef WEEX_PROJECT_WEEX_CONTEXT_QJS_H
#define WEEX_PROJECT_WEEX_CONTEXT_QJS_H
#include "core/runtime/weex_context.h"
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

class WeexContextQJS : public WeexContext {
 public:
  explicit WeexContextQJS(ScriptBridge *script_bridge) : WeexContext(script_bridge) {
    timer_function_id_ = 0;
  }

  void initGlobalContextFunctions() override;
  void initInstanceContextFunctions() override;
  void initFunctionForAppContext() override;
  void initFromParams(void *jsengine_vm,
                      std::vector<std::pair<std::string, std::string>> params,
                      bool forAppContext) override;
  void addExtraOptions(std::vector<std::pair<std::string, std::string>> params) override;
  void initWxEnvironment(std::vector<std::pair<std::string, std::string>> params,
                         bool forAppContext,
                         bool isSave) override;
  WeexContext *cloneWeexContext(const std::string &page_id,
                                bool initContext,
                                bool forAppContext) override;

  void updateInitFrameworkParams(const std::string &key, const std::string &value) override;
  void Release() override;

  void RunGC(void *engine_vm) override;

 public:
  class JSParams {

   public:
    explicit JSParams(JSContext *context, JSValue value, int type);
    void set_value(JSContext *context, JSValue value);

    ~JSParams();
    const char *value() {
      if (value_ == nullptr) {
        return "";
      }
      return value_;
    }

    const size_t size() {
      return length_;
    }

   public:
    static const int PARAMS_TYPE_JSON = 0;
    static const int PARAMS_TYPE_WSON = 1;

   private:
    int param_type;
    char *value_;
    size_t length_;
    JSContext *ctx_;
#if OS_ANDROID
    wson_buffer *wson_buffer_;
#endif
  };

  uint32_t gen_timer_function_id() {
    return ++timer_function_id_;
  }

  JSValue get_timer_function(uint32_t function_id) {
    auto iter = timer_function_.find(function_id);
    if (iter == timer_function_.end()) {
      return JS_UNDEFINED;
    }
    return timer_function_[function_id];
  }

  JSValue remove_timer(uint32_t function_id) {
    auto iter = timer_function_.find(function_id);
    if (iter == timer_function_.end()) {
      LOGE("timer do not exist!");
      return JS_UNDEFINED;
    }
    JSValue function = timer_function_[function_id];
    timer_function_.erase(function_id);
    return function;
  }

  inline void add_timer(uint32_t function_id, JSValue func) {
    timer_function_[function_id] = func;
  }

  ~WeexContextQJS() {
    timer_function_.clear();
    if (js_context()) {
      JSContext *context = static_cast<JSContext *>(js_context());
      JS_FreeContext(context);
    }
  }
 private:
  JSContext *createContext(JSRuntime *engine_vm);
  std::map<uint32_t, JSValue> timer_function_;
  uint32_t timer_function_id_;
};

#endif //WEEX_PROJECT_WEEX_CONTEXT_QJS_H
