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

#include "core/bridge/script/core_side_in_script.h"

#include <cstdlib>
#include "base/log_defines.h"
#include "base/make_copyable.h"
#include "base/thread/waitable_event.h"
#include "base/time_calculator.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "core/bridge/eagle_bridge.h"
#include "wson/wson_parser.h"
#include "core/config/core_environment.h"
#ifdef OS_ANDROID
#include "core/parser/action_args_check.h"
#include "android/weex_extend_js_api.h"
#endif

namespace WeexCore {

CoreSideInScript::CoreSideInScript() {}

CoreSideInScript::~CoreSideInScript() {}

void CoreSideInScript::CallNative(const char *page_id, const char *task,
                                  const char *callback) {
  if (page_id == nullptr || task == nullptr) return;
  std::string task_str(task);
  std::string target_str("[{\"module\":\"dom\",\"method\":\"createFinish\","
                         "\"args\":[]}]");
  std::string::size_type idx = task_str.find(target_str);

  if (idx == std::string::npos) {
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->CallNative(page_id, task, callback);
  } else {
    RenderManager::GetInstance()->CreateFinish(page_id);
  }
}

std::unique_ptr<ValueWithType> CoreSideInScript::CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  std::unique_ptr<ValueWithType> ret(new ValueWithType((int32_t) -1));
  if (page_id != nullptr && module != nullptr && method != nullptr) {
    return RenderManager::GetInstance()->CallNativeModule(page_id, module, method,
                                                          arguments, arguments_length,
                                                          options, options_length);
  }

  return ret;
}

void CoreSideInScript::CallNativeComponent(const char *page_id, const char *ref,
                                           const char *method,
                                           const char *arguments,
                                           int arguments_length,
                                           const char *options,
                                           int options_length) {
  if (page_id != nullptr && ref != nullptr && method != nullptr) {
    RenderManager::GetInstance()->CallNativeComponent(page_id,
                                                      ref,
                                                      method,
                                                      arguments,
                                                      arguments_length,
                                                      options,
                                                      options_length);
  }
}

void CoreSideInScript::AddElement(const char *page_id, const char *parent_ref,
                                  const char *dom_str, int dom_str_length,
                                  const char *index_str) {

//  std::string msg = "AddElement : ";
//  wson_parser parser(dom_str);
//  msg.append(parser.toStringUTF8().c_str());
//  weex::base::TimeCalculator
//      timeCalculator(weex::base::TaskPlatform::WEEXCORE, msg.c_str(), page_id);
  const char *indexChar = index_str == nullptr ? "\0" : index_str;
  int index = atoi(indexChar);
  if (page_id == nullptr || parent_ref == nullptr || dom_str == nullptr ||
      index < -1)
    return;

  std::string id(page_id);
  std::string pRef(parent_ref);
  char *temp = new char[dom_str_length + 1];
  memcpy(temp, dom_str, dom_str_length);
  temp[dom_str_length] = '\0';
  std::unique_ptr<char[]> dom;
  dom.reset(temp);

  WeexCoreManager::Instance()->script_thread()->message_loop()->PostTask(
      weex::base::MakeCopyable([page_id = id,
                                   parent_ref = pRef,
                                   dom_str = std::move(dom),
                                   i = index] {
        RenderManager::GetInstance()->AddRenderObject(page_id, parent_ref, i, dom_str.get());
      }));
}

void CoreSideInScript::SetTimeout(const char *callback_id, const char *time) {
  WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->SetTimeout(
      callback_id, time);
}

void CoreSideInScript::NativeLog(const char *str_array) {
  WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->NativeLog(
      str_array);
}

void CoreSideInScript::CreateBody(const char *page_id, const char *dom_str,
                                  int dom_str_length) {
  RenderManager::GetInstance()->CreatePage(page_id, dom_str);
}

int CoreSideInScript::UpdateFinish(const char *page_id, const char *task,
                                   int task_length, const char *callback,
                                   int callback_length) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->UpdateFinish(page_id, task, task_length, callback, callback_length);
}

void CoreSideInScript::CreateFinish(const char *page_id) {
  RenderManager::GetInstance()->CreateFinish(page_id);
}

int CoreSideInScript::RefreshFinish(const char *page_id, const char *task,
                                    const char *callback) {
  if (page_id == nullptr) return -1;
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->RefreshFinish(page_id, task, callback);
}

void CoreSideInScript::UpdateAttrs(const char *page_id, const char *ref,
                                   const char *data, int data_length) {

  RenderManager::GetInstance()->UpdateAttr(page_id, ref, data);
}

void CoreSideInScript::UpdateStyle(const char *page_id, const char *ref,
                                   const char *data, int data_length) {
  RenderManager::GetInstance()->UpdateStyle(page_id, ref, data);
}

void CoreSideInScript::RemoveElement(const char *page_id, const char *ref) {

  RenderManager::GetInstance()->RemoveRenderObject(page_id, ref);
}

void CoreSideInScript::MoveElement(const char *page_id, const char *ref,
                                   const char *parent_ref, int index) {

  RenderManager::GetInstance()->MoveRenderObject(page_id, ref, parent_ref,
                                                 index);
}

void CoreSideInScript::AddEvent(const char *page_id, const char *ref,
                                const char *event) {

  RenderManager::GetInstance()->AddEvent(page_id, ref, event);
}

void CoreSideInScript::RemoveEvent(const char *page_id, const char *ref,
                                   const char *event) {

  RenderManager::GetInstance()->RemoveEvent(page_id, ref, event);
}

const char *CoreSideInScript::CallGCanvasLinkNative(const char *context_id,
                                                    int type, const char *arg) {

#ifdef OS_ANDROID
  return CallGCanvasFun(context_id, type, arg);
#else
  return nullptr;
#endif
}

int CoreSideInScript::SetInterval(const char *page_id, const char *callback_id,
                                  const char *time) {
  return (atoi(page_id) << 16) | (atoi(callback_id));
}

void CoreSideInScript::ClearInterval(const char *page_id,
                                     const char *callback_id) {
  // do nothing
}

const char *CoreSideInScript::CallT3DLinkNative(int type, const char *arg) {
#ifdef OS_ANDROID
  return CallT3dFunc(type, arg);
#else
  return nullptr;
#endif
}

void CoreSideInScript::PostMessage(const char *vm_id, const char *data, int dataLength) {

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->PostMessage(vm_id, data, dataLength);
}

void CoreSideInScript::DispatchMessage(const char *client_id, const char *data, int dataLength,
                                       const char *callback,
                                       const char *vm_id) {

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->DispatchMessage(client_id, data, dataLength, callback, vm_id);
}

std::unique_ptr<WeexJSResult> CoreSideInScript::DispatchMessageSync(
    const char *client_id, const char *data, int dataLength,
    const char *vm_id) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->DispatchMessageSync(client_id, data, dataLength, vm_id);
}

void CoreSideInScript::ReportException(const char *page_id, const char *func,
                                       const char *exception_string) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->ReportException(page_id, func, exception_string);
}

void CoreSideInScript::SetJSVersion(const char *js_version) {
  LOGD("init JSFrm version %s", js_version);

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->SetJSVersion(js_version);
}

void CoreSideInScript::OnReceivedResult(long callback_id,
                                        std::unique_ptr<WeexJSResult> &result) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->OnReceivedResult(callback_id, result);
}

void CoreSideInScript::UpdateComponentData(const char *page_id,
                                           const char *cid,
                                           const char *json_data) {
  EagleBridge::GetInstance()->UpdateComponentData(page_id, cid, json_data);
}

bool CoreSideInScript::Log(int level, const char *tag,
                           const char *file,
                           unsigned long line,
                           const char *log) {
  return weex::base::LogImplement::getLog()->log((LogLevel) level, tag, file, line, log);
}
void CoreSideInScript::CompileQuickJSCallback(const char *key, const char *bytecode, int length) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()->CompileQuickJSCallback(key, bytecode, length);
}

}  // namespace WeexCore
