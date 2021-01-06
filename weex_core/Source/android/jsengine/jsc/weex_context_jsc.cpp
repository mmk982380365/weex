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
// Created by 董亚运 on 2020/8/7.
//

#include "weex_context_jsc.h"
void WeexContextJSC::initGlobalContextFunctions() {
  if (this->js_context() == nullptr) {
    return;
  }
  auto *globalObject = static_cast<WeexGlobalObject *> (this->js_context());
  globalObject->initGlobalContextFunctions();
}
void WeexContextJSC::initInstanceContextFunctions() {
  if (this->js_context() == nullptr) {
    return;
  }

  auto *globalObject = static_cast<WeexGlobalObject *> (this->js_context());
  globalObject->initInstanceContextFunctions();

}
void WeexContextJSC::initFunctionForAppContext() {
  if (this->js_context() == nullptr) {
    return;
  }

  auto *globalObject = static_cast<WeexGlobalObject *> (this->js_context());
  globalObject->initInstanceContextFunctions();
}
void WeexContextJSC::initFromParams(void *jsengine_vm, std::vector<std::pair<std::string, std::string>>params,
                                    bool forAppContext) {
  if (jsengine_vm == nullptr) {
    return;
  }

  VM *vm = static_cast<VM *> (jsengine_vm);
  JSLockHolder locker(vm);
  WeexGlobalObject
      *globalObject =
      WeexGlobalObject::create(*vm, WeexGlobalObject::createStructure(*vm, jsNull()));
  globalObject->initWxEnvironment(params, forAppContext, true, this->init_framework_args_);
  if (forAppContext)
    globalObject->initAppContextFunctions();
  else
    globalObject->initGlobalContextFunctions();
//  globalObject->timeQueue = this->timer_queue();

  SetJSContext(globalObject);
  jsc_context_strong_.set(*vm, globalObject);
  vm->heap.setGarbageCollectionTimerEnabled(true);
}
void WeexContextJSC::addExtraOptions(std::vector<std::pair<std::string, std::string>>params) {
  if (this->js_context() == nullptr) {
    return;
  }

  auto *globalObject = static_cast<WeexGlobalObject *> (this->js_context());
  globalObject->addExtraOptions(params);
}
void WeexContextJSC::initWxEnvironment(std::vector<std::pair<std::string, std::string>>params,
                                       bool forAppContext,
                                       bool isSave) {
  if (this->js_context() == nullptr) {
    return;
  }

  auto *globalObject = static_cast<WeexGlobalObject *> (this->js_context());
  globalObject->initWxEnvironment(params, forAppContext, isSave, this->init_framework_args_);
}
WeexContext *WeexContextJSC::cloneWeexContext(const std::string &page_id,
                                              bool initContext,
                                              bool forAppContext) {
  auto *weexContext = new WeexContextJSC(this->script_bridge());
  void *context = this->js_context();
  if (context == nullptr) {
    return nullptr;
  }
  auto *globalObject = static_cast<WeexGlobalObject *> (context);
  VM &vm = globalObject->vm();
  JSLockHolder locker(vm);
  auto *temp_object = WeexGlobalObject::create(vm,
                                               WeexGlobalObject::createStructure(vm, jsNull()));
  temp_object->SetScriptBridge(script_bridge());
  temp_object->initWxEnvironment(this->init_framework_args_,
                                 forAppContext,
                                 false,
                                 this->init_framework_args_);
  if (forAppContext)
    temp_object->initAppContextFunctions();
  else if (initContext)
    temp_object->initInstanceContextFunctions();
  else
    temp_object->initGlobalContextFunctions();
  temp_object->id = page_id;
  weexContext->page_id = page_id;

  weexContext->SetJSContext(temp_object);
  weexContext->jsc_context_strong_.set(vm, temp_object);
  return weexContext;
}
void WeexContextJSC::updateInitFrameworkParams(const std::string &key,
                                               const std::string &value) {
  if (this->js_context() == nullptr) {
    return;
  }

  auto *globalObject = static_cast<WeexGlobalObject *> (this->js_context());
  globalObject->updateInitFrameworkParams(key, value, init_framework_args_);
}
void WeexContextJSC::Release() {
  if (this->js_context() != nullptr) {
    auto *globalObject = static_cast<WeexGlobalObject *> (this->js_context());
    JSLockHolder locker(globalObject->vm());
    globalObject->vm().heap.collectAllGarbage();
  }
}
void WeexContextJSC::RunGC(void *engine_vm) {
  auto *globalObject = static_cast<WeexGlobalObject *>(js_context());
  ExecState *exec = globalObject->globalExec();
  JSLockHolder locker(exec);
  VM &vm = exec->vm();
  globalObject->resetPrototype(vm, jsNull());
  bool protectCountIsZero =
      Heap::heap(exec->vmEntryGlobalObject())->unprotect(exec->vmEntryGlobalObject());
  vm.heap.reportAbandonedObjectGraph();
  //vm.heap.collectSync(CollectionScope::Full);//collectAllGarbage();//collectAsync(CollectionScope::Full);
  //TODO disable gc temporary
  vm.heap.collectAllGarbage();
};
