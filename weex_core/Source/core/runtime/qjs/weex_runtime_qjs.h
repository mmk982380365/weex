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

#ifndef WEEX_PROJECT_WEEX_RUNTIME_QJS_H
#define WEEX_PROJECT_WEEX_RUNTIME_QJS_H
#include "core/runtime/weex_runtime.h"

class WeexRuntimeQJS : public WeexRuntime {
 public:
  explicit WeexRuntimeQJS(const std::shared_ptr<WeexCore::ScriptBridge> &script_bridge,
                          bool isMultiProgress = true);

  ~WeexRuntimeQJS() {};

  int initFramework(const std::string &script,
                    std::vector<std::pair<std::string, std::string>> params) override;

  int
  initAppFramework(const std::string &instanceId,
                   const std::string &appFramework,
                   std::vector<std::pair<std::string, std::string>> params) override;

  int createAppContext(const std::string &instanceId, const std::string &jsBundle) override;

  std::unique_ptr<WeexJSResult> exeJSOnAppWithResult(const std::string &instanceId,
                                                     const std::string &jsBundle) override;

  int callJSOnAppContext(const std::string &instanceId,
                         const std::string &func,
                         std::vector<VALUE_WITH_TYPE *> &params) override;

  int destroyAppContext(const std::string &instanceId) override;

  int exeJsService(const std::string &source) override;

  int exeCTimeCallback(const std::string &source) override;

  int exeJS(const std::string &instanceId,
            const std::string &nameSpace,
            const std::string &func,
            const std::vector<VALUE_WITH_TYPE *> &params) override;

  std::unique_ptr<WeexJSResult> exeJSWithResult(const std::string &instanceId,
                                                const std::string &nameSpace,
                                                const std::string &func,
                                                std::vector<VALUE_WITH_TYPE *> &params) override;

  void exeJSWithCallback(const std::string &instanceId,
                         const std::string &nameSpace,
                         const std::string &func,
                         std::vector<VALUE_WITH_TYPE *> &params,
                         long callback_id) override;

  int createInstance(const std::string &instanceId,
                     const std::string &func,
                     const char *script,
                     const int script_size,
                     const std::string &opts,
                     const std::string &initData,
                     const std::string &extendsApi,
                     std::vector<std::pair<std::string, std::string>> params,
                     unsigned int engine_type) override;

  std::unique_ptr<WeexJSResult> exeJSOnInstance(const std::string &instanceId,
                                                const char *script,
                                                const int script_size,
                                                unsigned int engine_type) override;

  int destroyInstance(const std::string &instanceId) override;

  int updateGlobalConfig(const std::string &config) override;

  int UpdateInitFrameworkParams(const std::string &key,
                                const std::string &value,
                                const std::string &desc) override;

  uint8_t *CompileQuickJSByteCode(const std::string &script, int *out_binary_size);

};

#endif //WEEX_PROJECT_WEEX_RUNTIME_QJS_H
