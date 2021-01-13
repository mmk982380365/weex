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
  class InstanceEngineData {
   public:
    explicit InstanceEngineData(std::string instanceId,
                                WeexRuntime *runtime,
                                JSEngineType engine_type,
                                bool forceInMainProcess,
                                bool backupThread,
                                bool preInitMode)
        : runtime_(runtime),
          engine_type_(engine_type),
          page_id_(instanceId),
          force_in_main_process_(forceInMainProcess),
          run_in_backup_thread_(backupThread),
          pre_init_mode_(preInitMode) {
    }

   public:
    inline WeexRuntime *runtime() { return runtime_; }
    inline JSEngineType engine_type() { return engine_type_; }
    inline std::string page_id() { return page_id_; }
    inline bool force_in_main_process() { return force_in_main_process_; }
    inline bool run_in_back_up_thread() { return run_in_backup_thread_; }
    inline bool pre_init_mode() { return pre_init_mode_; }

   private:
    WeexRuntime *runtime_;
    JSEngineType engine_type_;
    std::string page_id_;
    bool force_in_main_process_;
    bool run_in_backup_thread_;
    bool pre_init_mode_;
  };

 public:
  static WeexRuntimeManager *Instance() {
    if (instance == nullptr) {
      instance = new WeexRuntimeManager();
    }

    return instance;
  }

  WeexRuntime *default_runtime();

  WeexRuntimeManager::InstanceEngineData *instance_engine_data(std::string &page_id) {
    if (page_id.empty()) {
      return nullptr;
    }

    if (page_engine_type_map_.find(page_id) == page_engine_type_map_.end()) {
      return nullptr;
    }

    return page_engine_type_map_[page_id].get();
  }

  WeexRuntimeManager::InstanceEngineData *create_instance(std::string &page_id,
                                                          std::vector<std::pair<std::string,
                                                                                std::string>> &params);

  WeexRuntimeManager::InstanceEngineData *create_instance(std::string &page_id,
                                                          JSEngineType engine_type,
                                                          bool main_process_only,
                                                          bool backup_thread, bool pre_init_mode);

  inline void destroy_instance(std::string &page_id) {
    page_engine_type_map_.erase(page_id);
  }

  bool is_force_in_main_process(const std::string &page_id) {
    if (page_id.empty()) {
      return true;
    }

    if (page_engine_type_map_.find(page_id) == page_engine_type_map_.end()) {
      return false;
    }
    return page_engine_type_map_[page_id]->force_in_main_process();
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

  inline bool is_enable_main_process_script_side() {
    return this->is_enable_main_process_script_side_;
  }

  void set_enable_main_process_script_side(bool enableMainProcessManager) {
    this->is_enable_main_process_script_side_ = enableMainProcessManager;
  }

  std::map<JSEngineType, WeexRuntime *> runtime_from_page_id(const char *string);

 private:
  JSEngineType default_engine_type = ENGINE_JSC;
  std::map<JSEngineType, WeexRuntime *> runtime_map_;
  std::map<std::string, std::unique_ptr<InstanceEngineData>> page_engine_type_map_;

  unsigned int js_engine_type_;
  bool is_enable_backup_thread_;
  bool is_enable_backup_thread_cache_;
  bool is_enable_main_process_script_side_;

  static WeexRuntimeManager *instance;

  explicit WeexRuntimeManager() {
    default_engine_type = ENGINE_JSC;
    this->js_engine_type_ = default_engine_type;
    this->is_enable_backup_thread_ = false;
    this->is_enable_backup_thread_cache_ = false;
  }
};

#endif //WEEX_PROJECT_ENGINE_MANAGER_H
