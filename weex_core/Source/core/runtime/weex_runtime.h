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
// Created by Darin on 28/04/2018.
//

#ifndef WEEXV8_JSRUNTIME_H
#define WEEXV8_JSRUNTIME_H

#include "include/WeexApiHeader.h"
#include "weex_context.h"
#include "weex_context_holder.h"

namespace WeexCore {
class ScriptBridge;
}

enum JSEngineType {
  ENGINE_JSC = 1 << 0,
  ENGINE_QJS_BIN = 1 << 1,
  ENGINE_QJS = (1 << 2) | ENGINE_QJS_BIN
};

class WeexRuntime {

 public:
  explicit WeexRuntime(std::shared_ptr<WeexCore::ScriptBridge> script_bridge,
                       bool isMultiProgress = true) {
    this->script_bridge_ = script_bridge;
    this->is_multiprocess_ = isMultiProgress;
  };

  virtual ~WeexRuntime() {
  }

  bool hasInstanceId(const std::string &id) {
    return weex_context_holder_->has(id);
  };

  virtual int initFramework(const std::string &script,
                            std::vector<std::pair<std::string, std::string>> params) = 0;

  virtual int initAppFramework(const std::string &instanceId,
                               const std::string &appFramework,
                               std::vector<std::pair<std::string, std::string>> params) = 0;

  virtual int createAppContext(const std::string &instanceId, const std::string &jsBundle) = 0;

  virtual std::unique_ptr<WeexJSResult> exeJSOnAppWithResult(const std::string &instanceId,
                                                             const std::string &jsBundle) = 0;

  virtual int callJSOnAppContext(const std::string &instanceId,
                                 const std::string &func,
                                 std::vector<VALUE_WITH_TYPE *> &params) = 0;

  virtual int destroyAppContext(const std::string &instanceId) = 0;

  virtual int exeJsService(const std::string &source) = 0;

  virtual int exeCTimeCallback(const std::string &source) = 0;

  virtual int exeJS(const std::string &instanceId,
                    const std::string &nameSpace,
                    const std::string &func,
                    const std::vector<VALUE_WITH_TYPE *> &params) = 0;

  virtual std::unique_ptr<WeexJSResult> exeJSWithResult(const std::string &instanceId,
                                                        const std::string &nameSpace,
                                                        const std::string &func,
                                                        std::vector<VALUE_WITH_TYPE *> &params) = 0;

  virtual void exeJSWithCallback(const std::string &instanceId,
                                 const std::string &nameSpace,
                                 const std::string &func,
                                 std::vector<VALUE_WITH_TYPE *> &params,
                                 long callback_id) = 0;

  virtual int createInstance(const std::string &instanceId,
                             const std::string &func,
                             const char *script,
                             const int script_size,
                             const std::string &opts,
                             const std::string &initData,
                             const std::string &extendsApi,
                             std::vector<std::pair<std::string, std::string>> params,
                             unsigned int engine_type) = 0;

  virtual std::unique_ptr<WeexJSResult> exeJSOnInstance(const std::string &instanceId,
                                                        const char *script,
                                                        const int script_size,
                                                        unsigned int engine_type) = 0;

  virtual int destroyInstance(const std::string &instanceId) = 0;

  virtual int updateGlobalConfig(const std::string &config) = 0;

  virtual int UpdateInitFrameworkParams(const std::string &key,
                                        const std::string &value,
                                        const std::string &desc) = 0;

  template<class T>
  inline T *js_global_context(WeexContextHolder *holder) {
    if (holder == nullptr || holder->global_js_context() == nullptr) {
      return nullptr;
    }
    return static_cast<T *> (holder->global_js_context());
  }

  template<class T>
  inline T *weex_js_global_context() {
    return js_global_context<T>(weex_context_holder_.get());
  }

  template<class T>
  inline T *js_instance_context(WeexContext *weexContext) {
    if (weexContext == nullptr || weexContext->js_context() == nullptr) {
      return nullptr;
    }
    return static_cast<T *> (weexContext->js_context());
  }

  template<class T>
  T *find_js_instance_context(WeexContextHolder *holder, const std::string &page_id) {
    if (holder != nullptr) {
      void *js_context = holder->find_js_context(page_id);
      if (js_context != nullptr) {
        return static_cast<T *>(js_context);
      }
    }
    return nullptr;
  }

  template<class T>
  T *find_weex_js_instance_context(const std::string &page_id) {
    return find_js_instance_context<T>(weex_context_holder_.get(), page_id);
  }

  template<class T>
  inline T *engine_vm() {
    return static_cast<T *>(js_runtime_);
  }

  void set_js_runtime(void *js_runtime) {
    this->js_runtime_ = js_runtime;
  }

  void set_weex_context_holder(std::unique_ptr<WeexContextHolder> weex_context_holder) {
    weex_context_holder_ = std::move(weex_context_holder);
  }

  inline std::shared_ptr<WeexCore::ScriptBridge> script_bridge() {
    return script_bridge_;
  }

  inline WeexContextHolder *weex_context_holder() {
    return weex_context_holder_.get();
  }

  inline JSEngineType engine_type() { return this->js_engine_type_; }

  inline void set_engine_type(JSEngineType engine_type) {
    this->js_engine_type_ = engine_type;
  }

  WeexContextHolder *getLightAppContextHolder(const std::string &instanceId) {
    if (light_app_context_holder_map_.find(instanceId) == light_app_context_holder_map_.end()) {
      return nullptr;
    }
    return light_app_context_holder_map_[instanceId].get();
  }

  inline void bindLightAppContextHolder(const std::string &instanceId,
                                        WeexContextHolder *holder) {
    std::unique_ptr<WeexContextHolder> temp(holder);
    light_app_context_holder_map_[instanceId] = std::move(temp);
  }

  void unbindLightAppContextHolder(const std::string &instanceId) {
    light_app_context_holder_map_.erase(instanceId);
  }

  inline bool is_multi_process() {
    return is_multiprocess_;
  }

 private:
  JSEngineType js_engine_type_;
  std::unique_ptr<WeexContextHolder> weex_context_holder_;
  std::map<std::string, std::unique_ptr<WeexContextHolder>> light_app_context_holder_map_;
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge_;
  void *js_runtime_;
  bool is_multiprocess_;
};

#endif //WEEXV8_JSRUNTIME_H
