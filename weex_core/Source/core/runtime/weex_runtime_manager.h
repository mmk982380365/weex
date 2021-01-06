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

#ifndef WEEX_PROJECT_ENGINE_MANAGER_H
#define WEEX_PROJECT_ENGINE_MANAGER_H

#include <map>
#include "core/runtime/weex_runtime.h"

class WeexRuntimeManager {
 public:

  static WeexRuntimeManager *Instance() {
    if (instance == nullptr) {
      instance = new WeexRuntimeManager();
    }

    return instance;
  }

  WeexRuntime *default_runtime() ;

  void create_instance(std::string &page_id, JSEngineType engine_type) ;

  inline void destroy_instance(std::string &page_id) {
    page_engine_type_map_.erase(page_id);
  }

  WeexRuntime *find_runtime_from_engineType(JSEngineType engine_type);

  void add_weex_runtime(WeexRuntime *runtime) {
    runtime_map_[runtime->engine_type()] = runtime;
  }

  void set_engine_type(unsigned int jsEngineType) {
    this->js_engine_type_ = jsEngineType;
  }

  inline unsigned int supported_jsengine_type() {
    return this->js_engine_type_;
  }

  void set_enable_backup_thread(bool enableBackUpThread) {
    this->is_enable_backup_thread_ = enableBackUpThread;
  }

  inline bool is_enable_backup_thread() {
    return this->is_enable_backup_thread_;
  }

  void set_enable_backup_thread_cache(bool enableBackupThreadCache) {
    this->is_enable_backup_thread_cache_ = enableBackupThreadCache;
  }

  inline bool is_enable_backup_thread_cache() {
    return this->is_enable_backup_thread_cache_;
  }

  std::map<JSEngineType, WeexRuntime *> runtime_from_page_id(const char *string);
  class EngineData {
   public:
    explicit EngineData(WeexRuntime *runtime, JSEngineType engine_type) {
      this->runtime_ = runtime;
      this->engine_type_ = engine_type;
    }
    WeexRuntime *runtime_;
    JSEngineType engine_type_;
  };

 private:
  JSEngineType default_engine_type = ENGINE_JSC;

  std::map<JSEngineType, WeexRuntime *> runtime_map_;
  std::map<std::string, std::unique_ptr<EngineData>> page_engine_type_map_;
  unsigned int js_engine_type_;
  bool is_enable_backup_thread_;
  bool is_enable_backup_thread_cache_;

  static WeexRuntimeManager *instance;

  explicit WeexRuntimeManager() {
    default_engine_type = ENGINE_JSC;

    this->js_engine_type_ = default_engine_type;
    this->is_enable_backup_thread_ = false;
    this->is_enable_backup_thread_cache_ = false;
  }
};

#endif //WEEX_PROJECT_ENGINE_MANAGER_H
