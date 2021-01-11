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
// Created by 董亚运 on 2020/10/22.
//

#include "weex_runtime_manager.h"
#include "base/log_defines.h"
WeexRuntimeManager *WeexRuntimeManager::instance = nullptr;
std::map<JSEngineType,
         WeexRuntime *> WeexRuntimeManager::runtime_from_page_id(const char *page_id) {
  if (page_id == nullptr) {
    return this->runtime_map_;
  }

  std::string id(page_id);

  if (id.empty()) {
    return this->runtime_map_;
  }
  std::map<JSEngineType, WeexRuntime *> ret;
  auto it = page_engine_type_map_.find(id);
  if (it != page_engine_type_map_.end()) {
    ret[it->second->engine_type()] = it->second->runtime();
    return ret;
  }
  return ret;
}

WeexRuntime *WeexRuntimeManager::find_runtime_from_engineType(JSEngineType engine_type) {
  if ((js_engine_type_ & engine_type) == 0) {
    return nullptr;
  }

  auto it = runtime_map_.begin();
  while (it != runtime_map_.end()) {
    if (it->second->engine_type() & engine_type) {
      return it->second;
    }
    it++;
  }
  return nullptr;
}

WeexRuntime *WeexRuntimeManager::default_runtime() {
  auto it = runtime_map_.find(default_engine_type);
  if (it != runtime_map_.end()) {
    return it->second;
  }

  default_engine_type = default_engine_type == ENGINE_JSC ? ENGINE_QJS : ENGINE_JSC;
  //不可能既不是 jsc 也不是 qjs, 至少是一种
  it = runtime_map_.find(default_engine_type);
  return it->second;
}

WeexRuntimeManager::InstanceEngineData *WeexRuntimeManager::create_instance(std::string &page_id,
                                                                            JSEngineType engine_type,
                                                                            bool force_in_main_process,
                                                                            bool backup_thread,
                                                                            bool pre_init_mode) {
  JSEngineType ret = engine_type;
  LOGE("createInstance engine_type is %d", engine_type);
  WeexRuntime *runtime = find_runtime_from_engineType(engine_type);
  if (runtime == nullptr) {
    runtime = default_runtime();
    ret = runtime->engine_type();
  }

  page_engine_type_map_[page_id] =
      std::make_unique<InstanceEngineData>(page_id,
                                           runtime,
                                           ret,
                                           force_in_main_process,
                                           backup_thread, pre_init_mode);

  return page_engine_type_map_[page_id].get();
}
WeexRuntimeManager::InstanceEngineData *WeexRuntimeManager::create_instance(std::string &page_id,
                                                                            std::vector<std::pair<
                                                                                std::string,
                                                                                std::string>> &params) {
  JSEngineType engine_type = ENGINE_JSC;
  bool pre_init_mode = false;
  bool run_in_main_process_mode = false;
  bool run_in_backup_thread = false;
  for (const auto &param:params) {
    auto type = param.first;
    auto value = param.second;
    if (type == "use_back_thread") {
      if (value == "true") {
        run_in_backup_thread = true;
      }
    } else if (type == "engine_type") {
      LOGE("createInstance Set engine_type  %s", value.c_str());
      if (value == "QJS") {
        engine_type = ENGINE_QJS;
      } else if (value == "QJSBin") {
        engine_type = ENGINE_QJS_BIN;
      } else if (value == "JSC") {
        engine_type = ENGINE_JSC;
      }
    } else if (type == "pre_init_mode" && value == "true") {
      pre_init_mode = true;
    } else if (type == "run_in_main_process" && value == "true") {
      run_in_main_process_mode = true;
    }
  }
  run_in_main_process_mode = true;
  if (run_in_main_process_mode) {
    engine_type = ENGINE_QJS;
  }

  LOGE("dyyLog createInstance in mainProcess %d", run_in_main_process_mode);
  return create_instance(page_id,
                         engine_type,
                         run_in_main_process_mode,
                         run_in_backup_thread,
                         pre_init_mode);
}
