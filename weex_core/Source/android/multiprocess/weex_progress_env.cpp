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
// Created by Darin on 2018/7/22.
//

#include "android/utils/wson_jsc.h"

#include <memory>
#include "weex_progress_env.h"
#include "core/bridge/script_bridge.h"

namespace WeexCore {
class ScriptBridge;
}
WeexProgressEnv *WeexProgressEnv::env_ = nullptr;

WeexProgressEnv::WeexProgressEnv() {
}

void WeexProgressEnv::init_crash_handler(std::string crashFileName) {
  // initialize signal handler
  isMultiProcess = true;
  crashHandler = std::make_unique<crash_handler::CrashHandlerInfo>(crashFileName);
  crashHandler->initializeCrashHandler();
}
bool WeexProgressEnv::is_app_crashed() {
  if (!isMultiProcess)
    return false;
  bool crashed = crashHandler->is_crashed();
  if (crashed) {
    LOGE("jsEngine AppCrashed");
  }
  return crashed;
}
WeexProgressEnv::~WeexProgressEnv() {
  wson::destory();
}