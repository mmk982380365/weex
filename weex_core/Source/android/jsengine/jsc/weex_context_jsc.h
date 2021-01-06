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

#ifndef WEEX_PROJECT_WEEX_CONTEXT_JSC_H
#define WEEX_PROJECT_WEEX_CONTEXT_JSC_H

#include "core/runtime/weex_context.h"
#include "weex_utils_jsc.h"
#include "weex_global_object.h"

class WeexContextJSC : public WeexContext {
 public:
  explicit WeexContextJSC(ScriptBridge* script_bridge) : WeexContext(script_bridge) {
  }
  void initGlobalContextFunctions() override;
  void initInstanceContextFunctions() override;
  void initFunctionForAppContext() override;
  void initFromParams(void *jsengine_vm,
                      std::vector<std::pair<std::string, std::string>>params,
                      bool forAppContext) override;
  void addExtraOptions(std::vector<std::pair<std::string, std::string>>params) override;
  void initWxEnvironment(std::vector<std::pair<std::string, std::string>>params,
                         bool forAppContext,
                         bool isSave) override;

  //Only for GlobalWeexContext
  WeexContext *cloneWeexContext(const std::string &page_id,
                                bool initContext,
                                bool forAppContext) override;

  void updateInitFrameworkParams(const std::string &key, const std::string &value) override;

  ~WeexContextJSC() {
    jsc_context_strong_.clear();
  }

  void Release() override;

  void RunGC(void *engine_vm) override;

  RefPtr<VM> m_globalVM;

 private:
  Strong<WeexGlobalObject> jsc_context_strong_;
};

#endif //WEEX_PROJECT_WEEX_CONTEXT_JSC_H
