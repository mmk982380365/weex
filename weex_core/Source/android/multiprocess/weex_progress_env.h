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

#ifndef WEEXV8_WEEXENV_H
#define WEEXV8_WEEXENV_H

#include <mutex>
#include "base/crash/crash_handler.h"
#include "core/bridge/script_bridge.h"
#include <deque>
#include "weex_ipc_server.h"
#include "weex_ipc_client.h"
#include "third_party/IPC/Buffering/IPCBuffer.h"

class WeexProgressEnv {
 public:
  static WeexProgressEnv *getEnv() {
    static std::once_flag once_flag;
    std::call_once(once_flag, []() { env_ = new WeexProgressEnv(); });
    return env_;
  }

  WeexProgressEnv();
  ~WeexProgressEnv();
  bool enableTrace() const {
    return enableTrace_;
  }

  void setEnableTrace(bool enableTrace_) {
    WeexProgressEnv::enableTrace_ = enableTrace_;
  }

  inline void enableHandleAlarmSignal(bool enable) {
    crashHandler->setEnableAlarmSignal(enable);
  }


  void init_crash_handler(std::string crashFileName);

  bool is_app_crashed();

 public:
  volatile bool isMultiProcess = false;

 private:
  static WeexProgressEnv *env_;


  volatile bool enableTrace_;

  volatile bool m_cache_task_;

 private:

 private:
  std::unique_ptr<crash_handler::CrashHandlerInfo> crashHandler;
};

#endif //WEEXV8_WEEXENV_H
