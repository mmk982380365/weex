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
#include "core/runtime/qjs/weex_timer_manager_qjs.h"
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
  explicit WeexContextQJS(const std::shared_ptr<WeexCore::ScriptBridge> &script_bridge) : WeexContext(script_bridge) {
      timer_manager = std::shared_ptr<WeexTimerManagerQJS>(new WeexTimerManagerQJS);
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

  std::shared_ptr<WeexTimerManagerQJS> get_timer_manager(){
      return timer_manager;
  }


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



  ~WeexContextQJS() {
    if (js_context()) {
      JSContext *context = static_cast<JSContext *>(js_context());
      JS_FreeContext(context);
    }
    timer_manager->clear_timer();
  }
 private:
  JSContext *createContext(JSRuntime *engine_vm);
  std::shared_ptr<WeexTimerManagerQJS> timer_manager;

};

#endif //WEEX_PROJECT_WEEX_CONTEXT_QJS_H
