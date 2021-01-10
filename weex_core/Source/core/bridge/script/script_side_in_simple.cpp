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
// Created by 董亚运 on 2020/11/5.
//

#include "core/runtime/qjs/weex_runtime_qjs.h"
#include "core/runtime/qjs/weex_context_qjs.h"
#include "base/string_util.h"
#include "script_side_in_simple.h"

#if OS_ANDROID
#include "android/jsengine/jsc/weex_runtime_jsc.h"
#elif OS_IOS
#endif

int bridge::script::ScriptSideInSimple::InitFramework(const char *script,
                                                      std::vector<std::pair<std::string,
                                                                            std::string>> params) {
  // Engine Type
  unsigned int engine_type =
      main_process_only_ ? ENGINE_QJS : WeexRuntimeManager::Instance()->supported_jsengine_type();
  if (engine_type & ENGINE_JSC) {
    LOGE("Init JSC")
#if OS_ANDROID
    WeexRuntimeManager::Instance()
        ->add_weex_runtime(new WeexRuntimeJSC(this->bridge(), false));
#elif OS_IOS
#endif
  }

  if (engine_type & ENGINE_QJS) {
    LOGE("Init QJS")
    WeexRuntimeManager::Instance()
        ->add_weex_runtime(new WeexRuntimeQJS(this->bridge(), false));
  }

  auto runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id("");
  for (auto &runtime : runtime_map) {
    runtime.second->initFramework(script, params);
  }

  return 1;
}
int bridge::script::ScriptSideInSimple::InitAppFramework(const char *instanceId,
                                                         const char *appFramework,
                                                         std::vector<std::pair<std::string,
                                                                               std::string>> params) {
  return 1;
}
int bridge::script::ScriptSideInSimple::CreateAppContext(const char *instanceId,
                                                         const char *jsBundle) {
  return 1;
}
std::unique_ptr<WeexJSResult> bridge::script::ScriptSideInSimple::ExecJSOnAppWithResult(const char *instanceId,
                                                                                        const char *jsBundle) {
  return std::unique_ptr<WeexJSResult>();
}
int bridge::script::ScriptSideInSimple::CallJSOnAppContext(const char *instanceId,
                                                           const char *func,
                                                           std::vector<VALUE_WITH_TYPE *> &params) {
  return 1;
}
int bridge::script::ScriptSideInSimple::DestroyAppContext(const char *instanceId) {
  return 1;
}
int bridge::script::ScriptSideInSimple::ExecJsService(const char *source) {
  std::map<JSEngineType, WeexRuntime *>
      runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id("");
  for (auto &it : runtime_map) {
    it.second->exeJsService(source);
  }
  return 1;
}
int bridge::script::ScriptSideInSimple::ExecTimeCallback(const char *source) {
  return 1;
}
int bridge::script::ScriptSideInSimple::ExecJS(const char *instanceId,
                                               const char *nameSpace,
                                               const char *func,
                                               const std::vector<VALUE_WITH_TYPE *> &params) {
  std::map<JSEngineType, WeexRuntime *>
      runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id(instanceId);
  for (auto &runtime : runtime_map) {
    runtime.second->exeJS(weex::base::value_or_empty(instanceId),
                          weex::base::value_or_empty(nameSpace),
                          weex::base::value_or_empty(func),
                          params);
  }

  return 1;
}
std::unique_ptr<WeexJSResult> bridge::script::ScriptSideInSimple::ExecJSWithResult(const char *instanceId,
                                                                                   const char *nameSpace,
                                                                                   const char *func,
                                                                                   std::vector<
                                                                                       VALUE_WITH_TYPE *> &params) {
  std::map<JSEngineType, WeexRuntime *>
      runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id(instanceId);

  for (auto &runtime: runtime_map) {
    runtime.second->exeJSWithResult(weex::base::value_or_empty(instanceId),
                                    weex::base::value_or_empty(nameSpace),
                                    weex::base::value_or_empty(func), params);
  }

  return std::unique_ptr<WeexJSResult>();
}
void bridge::script::ScriptSideInSimple::ExecJSWithCallback(const char *instanceId,
                                                            const char *nameSpace,
                                                            const char *func,
                                                            std::vector<VALUE_WITH_TYPE *> &params,
                                                            long callback_id) {
  std::map<JSEngineType, WeexRuntime *>
      runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id(instanceId);

  for (auto &runtime: runtime_map) {
    runtime.second->exeJSWithCallback(weex::base::value_or_empty(instanceId),
                                      weex::base::value_or_empty(nameSpace),
                                      weex::base::value_or_empty(func), params, callback_id);
  }
}
int bridge::script::ScriptSideInSimple::CreateInstance(const char *instanceId,
                                                       const char *func,
                                                       const char *script,
                                                       const int script_size,
                                                       const char *opts,
                                                       const char *initData,
                                                       const char *extendsApi,
                                                       std::vector<std::pair<std::string,
                                                                             std::string>> params) {
  std::string page_id(weex::base::value_or_empty(instanceId));
  auto instanceData = WeexRuntimeManager::Instance()->instance_engine_data(page_id);
  if (instanceData == nullptr) {
    instanceData = WeexRuntimeManager::Instance()->create_instance(page_id, params);
  }

  JSEngineType engine_type = instanceData->engine_type();
  if (instanceData->pre_init_mode() && instanceData->engine_type() == ENGINE_QJS_BIN) {
    //无法控制 pre_init_mode 的 script,
    // 所以这里还是走 source code 的执行方式,
    // 但 pre_init_mode 下 exeJsOnInstance 还是要走 bytecode 模式
    engine_type = ENGINE_QJS;
  }

  instanceData->runtime()->createInstance(page_id,
                                          weex::base::value_or_empty(func),
                                          script,
                                          script_size,
                                          weex::base::value_or_empty(opts),
                                          weex::base::value_or_empty(initData),
                                          weex::base::value_or_empty(extendsApi),
                                          params, engine_type);
  return 1;
}
std::unique_ptr<WeexJSResult> bridge::script::ScriptSideInSimple::ExecJSOnInstance(const char *instanceId,
                                                                                   const char *script,
                                                                                   const int script_size,
                                                                                   int type) {
  std::map<JSEngineType, WeexRuntime *>
      runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id(instanceId);

  for (auto &runtime: runtime_map) {
    runtime.second->exeJSOnInstance(weex::base::value_or_empty(instanceId),
                                    script,
                                    script_size,
                                    runtime.first);
  }

  return std::unique_ptr<WeexJSResult>();
}
int bridge::script::ScriptSideInSimple::DestroyInstance(const char *instanceId) {

  std::string page_id(weex::base::value_or_empty(instanceId));

  std::map<JSEngineType, WeexRuntime *>
      runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id(instanceId);
  for (auto &runtime: runtime_map) {
    runtime.second->destroyInstance(page_id);
  }
  WeexRuntimeManager::Instance()->destroy_instance(page_id);
  return 1;
}
int bridge::script::ScriptSideInSimple::UpdateGlobalConfig(const char *config) {

  std::map<JSEngineType, WeexRuntime *>
      runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id("");
  for (auto &runtime : runtime_map) {
    runtime.second->updateGlobalConfig(config);
  }

  return 1;
}
int bridge::script::ScriptSideInSimple::UpdateInitFrameworkParams(const std::string &key,
                                                                  const std::string &value,
                                                                  const std::string &desc) {

  std::map<JSEngineType, WeexRuntime *>
      runtime_map = WeexRuntimeManager::Instance()->runtime_from_page_id("");
  for (auto &runtime : runtime_map) {
    runtime.second->UpdateInitFrameworkParams(key, value, desc);
  }

  return 1;
}
void bridge::script::ScriptSideInSimple::SetLogType(const int logLevel, const bool isPerf) {

}
void bridge::script::ScriptSideInSimple::CompileQuickJSBin(const char *key, const char *script) {
  WeexRuntime *kPRuntime = WeexRuntimeManager::Instance()->find_runtime_from_engineType(ENGINE_QJS);
  if (kPRuntime != nullptr && kPRuntime->engine_type() == ENGINE_QJS) {
    int length = 0;
    uint8_t *bytecode =
        static_cast<WeexRuntimeQJS *>(kPRuntime)->CompileQuickJSByteCode(script, &length);
    bridge()->core_side()->CompileQuickJSCallback(key,
                                                  reinterpret_cast<const char *>(bytecode),
                                                  length);
  }

}
