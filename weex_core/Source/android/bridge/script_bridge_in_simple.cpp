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

#include "android/bridge/script_bridge_in_simple.h"
#include "core/bridge/script/core_side_in_script.h"
#include "core/bridge/script/script_side_in_simple.h"

ScriptBridgeInSimple::ScriptBridgeInSimple() {
  set_script_side(new bridge::script::ScriptSideInSimple(true));
  set_core_side(new CoreSideInScript);
}
ScriptBridgeInSimple::~ScriptBridgeInSimple() {

}
