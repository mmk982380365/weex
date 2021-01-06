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
#include "weex_context_holder.h"
//#include "android/multiprocess/weex_progress_env.h"

void WeexContextHolder::initFromParams(std::vector<std::pair<std::string, std::string>> params,
                                       bool forAppContext) {
  return weex_global_context_->initFromParams(js_engine_vm_, params, forAppContext);
}

WeexContextHolder::WeexContextHolder(void *vm,
                                     WeexContext *context,
                                     bool isMultiProgress) {
  this->isMultiProgress = isMultiProgress;
  this->js_engine_vm_ = vm;
  this->weex_global_context_ = context;
}

WeexContext *WeexContextHolder::cloneWeexObject(const std::string &page_id,
                                                bool initContext,
                                                bool forAppContext) {
  return weex_global_context_->cloneWeexContext(page_id, initContext, forAppContext);
}

WeexContextHolder::~WeexContextHolder() {
  for (const auto&context : weex_context_map) {
    delete context.second;
  }
  weex_context_map.clear();
  if (weex_global_context_) {
    weex_global_context_->Release();
  }
}

