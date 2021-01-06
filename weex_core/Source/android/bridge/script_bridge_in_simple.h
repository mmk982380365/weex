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

#ifndef WEEX_PROJECT_WEEX_CORE_SOURCE_ANDROID_BRIDGE_SCRIPT_BRIDGE_IN_SIMPILE_H_
#define WEEX_PROJECT_WEEX_CORE_SOURCE_ANDROID_BRIDGE_SCRIPT_BRIDGE_IN_SIMPILE_H_
#include "core/bridge/script_bridge.h"
namespace WeexCore {
class ScriptBridgeInSimple : public ScriptBridge {
 public:
  ScriptBridgeInSimple();
  ~ScriptBridgeInSimple();
 private:
  DISALLOW_COPY_AND_ASSIGN(ScriptBridgeInSimple);
};
}

#endif //WEEX_PROJECT_WEEX_CORE_SOURCE_ANDROID_BRIDGE_SCRIPT_BRIDGE_IN_SIMPILE_H_
