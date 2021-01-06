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
    ret[it->second->engine_type_] = it->second->runtime_;
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

void WeexRuntimeManager::create_instance(std::string &page_id, JSEngineType engine_type) {
  JSEngineType ret = engine_type;
  LOGE("createInstance engine_type is %d", engine_type);
  WeexRuntime *runtime = find_runtime_from_engineType(engine_type);
  if (runtime == nullptr) {
    runtime = default_runtime();
    ret = runtime->engine_type();
  }
  page_engine_type_map_[page_id] = std::make_unique<EngineData>(runtime, ret);
}
