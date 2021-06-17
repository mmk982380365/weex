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
// Created by 董亚运 on 2020/8/7.
//

#ifndef WEEX_PROJECT_WEEX_CONTEXT_H
#define WEEX_PROJECT_WEEX_CONTEXT_H

#include "include/WeexApiHeader.h"
#include <string>

namespace WeexCore {
class ScriptBridge;
}

class WeexContext {
 public:
  explicit WeexContext(std::shared_ptr<WeexCore::ScriptBridge> script_bridge) {
    this->script_bridge_ = script_bridge;
  }

  virtual void initGlobalContextFunctions() = 0;
  virtual void initInstanceContextFunctions() = 0;
  virtual void initFunctionForAppContext() = 0;
  virtual void initFromParams(void *jsengine_vm,
                              std::vector<std::pair<std::string, std::string>>params,
                              bool forAppContext) = 0;
  virtual void addExtraOptions(std::vector<std::pair<std::string, std::string>>params) = 0;
  virtual void initWxEnvironment(std::vector<std::pair<std::string, std::string>>params,
                                 bool forAppContext,
                                 bool isSave) = 0;
  virtual WeexContext *cloneWeexContext(const std::string &page_id,
                                        bool initContext,
                                        bool forAppContext) = 0;

  virtual void updateInitFrameworkParams(const std::string &key, const std::string &value) = 0;
  virtual void Release() = 0;

  virtual void RunGC(void *engine_vm) = 0;

  virtual ~WeexContext() { this->js_context_ = nullptr; };
  inline std::shared_ptr<WeexCore::ScriptBridge> script_bridge() { return script_bridge_; }

  inline void *js_context() { return js_context_; }
  void SetJSContext(void *js_context) {
    this->js_context_ = js_context;
  }

 public:
  std::vector<std::pair<std::string, std::string>> init_framework_args_;
  std::string page_id = "";

 private:
  void *js_context_;
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge_;
};

#endif //WEEX_PROJECT_WEEX_CONTEXT_H
