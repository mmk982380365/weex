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
// Created by Darin on 10/04/2018.
//

#ifndef WEEXV8_WEEXOBJECTHOLDER_H
#define WEEXV8_WEEXOBJECTHOLDER_H

#include "weex_context.h"
class WeexContextHolder {
 public:
  ~WeexContextHolder();
  explicit WeexContextHolder(void *vm,
                             WeexContext *context,
                             bool isMultiProgress);

  void initFromParams(std::vector<std::pair<std::string, std::string>>params, bool forAppContext);

  WeexContext *cloneWeexObject(const std::string &page_id, bool initContext, bool forAppContext);

  inline WeexContext *global_weex_context() {
    return this->weex_global_context_;
  }

  void *find_js_context(const std::string &page_id) {
    auto iterator = weex_context_map.find(page_id);
    if (iterator != weex_context_map.end()) {
      WeexContext *weexContext = weex_context_map[page_id];
      if (weexContext != nullptr) {
        return weexContext->js_context();
      }
    }
    return nullptr;
  }

  inline void *global_js_context() {
    if (this->weex_global_context_ == nullptr) {
      return nullptr;
    }
    return this->weex_global_context_->js_context();
  }

  inline void put(const std::string &page_id, WeexContext *context) {
    this->weex_context_map[page_id] = context;
  }

  inline void erase(const std::string &page_id) {

    auto iterator = weex_context_map.find(page_id);
    if (iterator != weex_context_map.end()) {
      WeexContext *weexContext = weex_context_map[page_id];
      this->weex_context_map.erase(page_id);
      if (page_id.length() > 0) {
        int index = atoi(page_id.c_str());
        if (index > 0 && index % 20 == 0) {
          weexContext->RunGC(js_engine_vm_);
        }
      }
      delete weexContext;
      weexContext = nullptr;
    }
  }

  bool has(const std::string &page_id) {
    return weex_context_map.find(page_id) != weex_context_map.end();
  }

 private:
  std::map<std::string, WeexContext *> weex_context_map;
  void *js_engine_vm_;
  WeexContext *weex_global_context_;
  bool isMultiProgress;
};

#endif //WEEXV8_WEEXOBJECTHOLDER_H
