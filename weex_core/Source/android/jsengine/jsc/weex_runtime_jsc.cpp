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
// Created by 董亚运 on 2020/8/6.
//

#include "weex_runtime_jsc.h"
#include "android/multiprocess/bridge/script/core_side_in_multi_process.h"
#include "android/multiprocess/weex_progress_env.h"
#include "weex_utils_jsc.h"
#include "weex_context_jsc.h"
#include "include/JavaScriptCore/runtime/Exception.h"
#include "include/JavaScriptCore/heap/HeapSnapshotBuilder.h"
#include <memory>
#include <base/time_calculator.h>

using namespace JSC;
using namespace WTF;
using namespace WEEXICU;

static void _getArgListFromJSParams(MarkedArgumentBuffer *obj,
                                    ExecState *state,
                                    const std::vector<VALUE_WITH_TYPE *> &params);

WeexRuntimeJSC::WeexRuntimeJSC(std::shared_ptr<WeexCore::ScriptBridge> script_bridge,
                               bool isMultiProgress) : WeexRuntime(script_bridge, isMultiProgress) {
  set_engine_type(JSEngineType::ENGINE_JSC);
  initJSC(isMultiProgress);
  auto *weexContextJsc = new WeexContextJSC(script_bridge);
  weexContextJsc->m_globalVM = std::move(VM::create(LargeHeap));
  set_js_runtime(weexContextJsc->m_globalVM.get());
  static bool wson_init_flag = false;
  if (!wson_init_flag) {
    wson_init_flag = true;
    wson::init(weexContextJsc->m_globalVM.get());
  }
  set_weex_context_holder(std::make_unique<WeexContextHolder>(weexContextJsc->m_globalVM.get(),
                                                              weexContextJsc,
                                                              isMultiProgress));
  LOGE("WeexRuntime is running and mode is %s", isMultiProgress ? "multiProcess" : "singleProcess");
}

int WeexRuntimeJSC::initFramework(const std::string &script,
                                  std::vector<std::pair<std::string, std::string>> params) {
  base::debug::TraceEvent::StartATrace(nullptr);
  base::debug::TraceScope traceScope("weex", "initFramework");
  weex_context_holder()->initFromParams(params, false);
  return _initFramework(script);
}

int WeexRuntimeJSC::initAppFramework(const std::string &instanceId, const std::string &appFramework,
                                     std::vector<std::pair<std::string, std::string>> params) {
// Create Worker Context
  LOGE("Weex jsserver initAppFramework %s", instanceId.c_str());
  auto holder = getLightAppContextHolder(instanceId);
  if (holder == nullptr) {
    holder = new WeexContextHolder(engine_vm<VM>(),
                                   new WeexContextJSC(script_bridge()),
                                   this->is_multi_process());
    LOGE("Weex jsserver initAppFramework pHolder == null and id %s", instanceId.c_str());
    bindLightAppContextHolder(instanceId, holder);
  }

  holder->initFromParams(params, true);
  return _initAppFramework(instanceId, appFramework);
}

int WeexRuntimeJSC::_initAppFramework(const std::string &instanceId,
                                      const std::string &appFramework) {
  JSLockHolder locker_global(engine_vm<VM>());

  auto appWorkerObjectHolder = getLightAppContextHolder(instanceId);
  if (appWorkerObjectHolder == nullptr) {
    return static_cast<int32_t>(false);
  }
  WeexGlobalObject
      *worker_globalObject = js_global_context<WeexGlobalObject>(appWorkerObjectHolder);
  worker_globalObject->SetScriptBridge(script_bridge());

  worker_globalObject->id = instanceId;
  return static_cast<int32_t>(
      ExecuteJavaScript(worker_globalObject,
                        String::fromUTF8(appFramework.c_str()),
                        "(app framework)",
                        true,
                        "initAppFramework",
                        instanceId.c_str()));
}

int WeexRuntimeJSC::createAppContext(const std::string &instanceId, const std::string &jsBundle) {
  if (instanceId.empty()) {
    return static_cast<int32_t>(false);
  } else {
    std::string pre = "";
    if (instanceId.length() > 6) {
      pre = instanceId.substr(0, 7);
    }

    std::string get_context_fun_name;
    std::string final_instanceId;
    if (pre == "plugin_") {
      LOGE("createAppContext __get_plugin_context__");
      get_context_fun_name = "__get_plugin_context__";
      final_instanceId = instanceId.substr(7, instanceId.length() - 7);
    } else {
      LOGE("createAppContext __get_app_context__");
      get_context_fun_name = "__get_app_context__";
      final_instanceId = instanceId;
    }

    // new a global object
    // --------------------------------------------------
    auto appWorkerObjectHolder = getLightAppContextHolder(final_instanceId);
    if (appWorkerObjectHolder == nullptr) {
      return static_cast<int32_t>(false);
    }

    JSGlobalObject
        *worker_globalObject = js_global_context<WeexGlobalObject>(appWorkerObjectHolder);
    if (worker_globalObject == nullptr) {
      return static_cast<int32_t>(false);
    }

    JSLockHolder locker_global(engine_vm<VM>());

    WeexGlobalObject
        *app_globalObject =
        js_instance_context<WeexGlobalObject>(appWorkerObjectHolder->cloneWeexObject(
            final_instanceId,
            true,
            true));
    weex::GlobalObjectDelegate *delegate = NULL;
    app_globalObject->SetScriptBridge(script_bridge());
//        VM &vm = worker_globalObject->vm();
//
//        JSLockHolder locker_1(&vm);
//
//        VM &thisVm = app_globalObject->vm();
//        JSLockHolder locker_2(&thisVm);
//
    PropertyName createInstanceContextProperty
        (Identifier::fromString(engine_vm<VM>(), get_context_fun_name.c_str()));
    ExecState *state = worker_globalObject->globalExec();
    JSValue createInstanceContextFunction =
        worker_globalObject->get(state, createInstanceContextProperty);
    MarkedArgumentBuffer args;

    CallData callData;
    CallType callType = getCallData(createInstanceContextFunction, callData);
    NakedPtr<Exception> returnedException;
    JSValue ret = call(state, createInstanceContextFunction, callType, callData,
                       worker_globalObject, args, returnedException);
    if (returnedException) {
      String exceptionInfo = exceptionToString(worker_globalObject, returnedException->value());
      return static_cast<int32_t>(false);
    }
    app_globalObject->resetPrototype(*(engine_vm<VM>()), ret);
    app_globalObject->id = final_instanceId;
    // --------------------------------------------------


    if (!ExecuteJavaScript(app_globalObject,
                           String::fromUTF8(jsBundle.c_str()),
                           ("weex createAppContext"),
                           true,
                           "createAppContext",
                           final_instanceId.c_str())) {
      return static_cast<int32_t>(false);
    }
  }
  return static_cast<int32_t>(true);
}

std::unique_ptr<WeexJSResult> WeexRuntimeJSC::exeJSOnAppWithResult(const std::string &instanceId,
                                                                   const std::string &jsBundle) {

  std::unique_ptr<WeexJSResult> returnResult;
  return returnResult;
}

int
WeexRuntimeJSC::callJSOnAppContext(const std::string &instanceId,
                                   const std::string &func,
                                   std::vector<VALUE_WITH_TYPE *> &params) {
  if (instanceId == "") {
    return static_cast<int32_t>(false);
  } else {
    auto appWorkerObjectHolder = getLightAppContextHolder(instanceId);
    if (appWorkerObjectHolder == nullptr) {
//            LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT weexLiteAppObjectHolder is null");
      return static_cast<int32_t>(false);
    }

    JSGlobalObject
        *worker_globalObject = js_global_context<WeexGlobalObject>(appWorkerObjectHolder);
    if (worker_globalObject == NULL) {
//            LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT worker_globalObject is null");
      return static_cast<int32_t>(false);
    }
//        LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT1");
    JSLockHolder locker_global(engine_vm<VM>());

//        LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT2");
    MarkedArgumentBuffer obj;
    ExecState *state = worker_globalObject->globalExec();
    _getArgListFromJSParams(&obj, state, params);
//        LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT3");
    Identifier funcIdentifier = Identifier::fromString(engine_vm<VM>(), func.c_str());

    JSValue function;
    JSValue result;
    function = worker_globalObject->get(state, funcIdentifier);
    CallData callData;
    CallType callType = getCallData(function, callData);
    NakedPtr<Exception> returnedException;
    // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT start call js runtime funtion");
    if (function.isEmpty()) {
      LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT js funtion is empty");
    }
    JSValue ret =
        call(state, function, callType, callData, worker_globalObject, obj, returnedException);
    // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT end");
    if (returnedException) {
      ReportException(worker_globalObject,
                      returnedException.get(),
                      instanceId.c_str(),
                      func.c_str());
      return static_cast<int32_t>(false);
    }
    worker_globalObject->vm().drainMicrotasks();
    return static_cast<int32_t>(true);
  }
}

int WeexRuntimeJSC::destroyAppContext(const std::string &instanceId) {
  auto appWorkerObjectHolder = getLightAppContextHolder(instanceId);
  if (appWorkerObjectHolder == nullptr) {
    return static_cast<int32_t>(false);
  }

  LOGE("Weex jsserver IPCJSMsg::DESTORYAPPCONTEXT end1 %s", instanceId.c_str());
  std::map<std::string, WeexContext *>::iterator it_find_instance;

  WeexGlobalObject
      *globalObject = find_js_instance_context<WeexGlobalObject>(appWorkerObjectHolder, instanceId);
  if (globalObject != nullptr) {
    // LOGE("Weex jsserver IPCJSMsg::DESTORYAPPCONTEXT mAppInstanceGlobalObjectMap donnot contain and return");
    appWorkerObjectHolder->erase(instanceId);
  }

//   GC on VM
//    WeexGlobalObject* instanceGlobalObject = mAppInstanceGlobalObjectMap[instanceId.utf8().data()];
//    if (instanceGlobalObject == NULL) {
//      return static_cast<int32_t>(true);
//    }
//    LOGE("Weex jsserver IPCJSMsg::DESTORYAPPCONTEXT start GC");
//    VM& vm_global = *globalVM.get();
//    JSLockHolder locker_global(&vm_global);
//
//    ExecState* exec_ = instanceGlobalObject->globalExec();
//    JSLockHolder locker_(exec_);
//    VM& vm_ = exec_->vm();
//    vm_.heap.collectAllGarbage();
//    instanceGlobalObject->m_server = nullptr;
//    instanceGlobalObject = NULL;

//  appWorkerContextHolderMap.erase(instanceId);
  unbindLightAppContextHolder(instanceId);
  delete appWorkerObjectHolder;
  appWorkerObjectHolder = nullptr;
  // LOGE("mAppInstanceGlobalObjectMap size:%d mAppGlobalObjectMap size:%d",
  //      mAppInstanceGlobalObjectMap.size(), mAppGlobalObjectMap.size());
  return static_cast<int32_t>(true);
}

int WeexRuntimeJSC::exeJsService(const std::string &source) {
  base::debug::TraceScope traceScope("weex", "exeJSService");
  JSGlobalObject *globalObject = weex_js_global_context<WeexGlobalObject>();
  JSLockHolder locker(engine_vm<VM>());
  if (!ExecuteJavaScript(globalObject,
                         String::fromUTF8(source.c_str()),
                         ("weex service"),
                         true,
                         "execjsservice")) {
    LOGE("jsLog JNI_Error >>> scriptStr :%s", source.c_str());
    return static_cast<int32_t>(false);
  }
  return static_cast<int32_t>(true);
}

int WeexRuntimeJSC::exeCTimeCallback(const std::string &source) {
  base::debug::TraceScope traceScope("weex", "EXECTIMERCALLBACK");
//    LOGE("IPC EXECTIMERCALLBACK and ExecuteJavaScript");
  JSGlobalObject *globalObject = weex_js_global_context<WeexGlobalObject>();
  JSLockHolder locker(engine_vm<VM>());
  if (!ExecuteJavaScript(globalObject,
                         String::fromUTF8(source.c_str()),
                         ("weex service"),
                         false,
                         "timercallback")) {
    LOGE("jsLog EXECTIMERCALLBACK >>> scriptStr :%s", source.c_str());
    return static_cast<int32_t>(false);
  }

  return static_cast<int32_t>(true);
}

int WeexRuntimeJSC::exeJS(const std::string &instanceId,
                          const std::string &nameSpace,
                          const std::string &func,
                          const std::vector<VALUE_WITH_TYPE *> &params) {
//    LOGE("EXECJS func:%s and params size is %d", func.utf8().data(), params.size());

  std::string runFunc = func;
  JSGlobalObject *globalObject = nullptr;
  // fix instanceof Object error
  // if function is callJs on instance, should us Instance object to call __WEEX_CALL_JAVASCRIPT__

  if (std::strcmp("callJS", runFunc.c_str()) == 0) {

    globalObject =
        find_weex_js_instance_context<WeexGlobalObject>(instanceId);

    if (globalObject == nullptr) {
      globalObject = weex_js_global_context<WeexGlobalObject>();
    } else {
      runFunc = "__WEEX_CALL_JAVASCRIPT__";
    }
  } else {
    globalObject = weex_js_global_context<WeexGlobalObject>();
  }
  JSLockHolder locker(engine_vm<VM>());

  MarkedArgumentBuffer obj;
  base::debug::TraceScope traceScope("weex", "exeJS", "function", runFunc.c_str());
  ExecState *state = globalObject->globalExec();
  _getArgListFromJSParams(&obj, state, params);

  Identifier funcIdentifier = Identifier::fromString(engine_vm<VM>(), runFunc.c_str());

  JSValue function;
  JSValue result;
  if (nameSpace.empty()) {
    function = globalObject->get(state, funcIdentifier);
  } else {
    Identifier
        namespaceIdentifier = Identifier::fromString(engine_vm<VM>(), nameSpace.c_str());
    JSValue master = globalObject->get(state, namespaceIdentifier);
    if (!master.isObject()) {
      return static_cast<int32_t>(false);
    }
    function = master.toObject(state)->get(state, funcIdentifier);
  }
  CallData callData;
  CallType callType = getCallData(function, callData);
  NakedPtr<Exception> returnedException;
  JSValue ret = call(state, function, callType, callData, globalObject, obj, returnedException);

  engine_vm<VM>()->drainMicrotasks();

  if (returnedException) {
    ReportException(globalObject, returnedException.get(), instanceId.c_str(), func.c_str());
    return static_cast<int32_t>(false);
  }
  return static_cast<int32_t>(true);
}

inline void convertJSArrayToWeexJSResult(ExecState *state, JSValue &ret, WeexJSResult *jsResult) {
  if (ret.isUndefined() || ret.isNull() || !isJSArray(ret)) {
    // createInstance return whole source object, which is big, only accept array result
    return;
  }
  //
  /** most scene, return result is array of null */
  JSArray *array = asArray(ret);
  uint32_t length = array->length();
  bool isAllNull = true;
  for (uint32_t i = 0; i < length; i++) {
    JSValue ele = array->getIndex(state, i);
    if (!ele.isUndefinedOrNull()) {
      isAllNull = false;
      break;
    }
  }
  if (isAllNull) {
    return;
  }
  const char *data;
//  if (WeexProgressEnv::getEnv()->useWson()) { //Wson Always True
  if (true) {
    wson_buffer *buffer = wson::toWson(state, ret);
    data = (char *) buffer->data;
    jsResult->length = buffer->position;
    buffer->data = nullptr;
    wson_buffer_free(buffer);
  } else {
    String string = JSONStringify(state, ret, 0);
    CString cstring = string.utf8();
    char *buf = new char[cstring.length() + 1];
    memcpy(buf, cstring
        .
            data(), cstring
               .
                   length()
    );
    buf[cstring.
        length()
    ] = '\0';
    jsResult->
        length = cstring.length();
    data = cstring.data();
  }
  char *buf = new char[jsResult->length + 1];
  memcpy(buf, data, jsResult
      ->length);
  buf[jsResult->length] = '\0';
  jsResult->data.
      reset(buf);
}

std::unique_ptr<WeexJSResult> WeexRuntimeJSC::exeJSWithResult(const std::string &instanceId,
                                                              const std::string &nameSpace,
                                                              const std::string &func,
                                                              std::vector<VALUE_WITH_TYPE *> &params) {

  std::unique_ptr<WeexJSResult> returnResult;
  returnResult.reset(new WeexJSResult);

  JSGlobalObject *globalObject;
  std::string runFunc = func;
  // fix instanceof Object error
  // if function is callJs should us Instance object to call __WEEX_CALL_JAVASCRIPT__
  if (std::strcmp("callJS", runFunc.c_str()) == 0) {
    globalObject =
        find_weex_js_instance_context<WeexGlobalObject>(instanceId);
    if (globalObject == nullptr) {
      globalObject = weex_js_global_context<WeexGlobalObject>();
    } else {
      runFunc = "__WEEX_CALL_JAVASCRIPT__";
    }
  } else {
    globalObject = weex_js_global_context<WeexGlobalObject>();
  }
  VM &vm = globalObject->vm();
  JSLockHolder locker(&vm);

  base::debug::TraceScope traceScope("weex", "exeJSWithResult", "function", runFunc.c_str());

  MarkedArgumentBuffer obj;
  ExecState *state = globalObject->globalExec();

  _getArgListFromJSParams(&obj, state, params);

  Identifier funcIdentifier = Identifier::fromString(&vm, runFunc.c_str());
  JSValue function;
  JSValue result;
  if (nameSpace.empty()) {
    function = globalObject->get(state, funcIdentifier);
  } else {
    Identifier namespaceIdentifier = Identifier::fromString(&vm, nameSpace.c_str());
    JSValue master = globalObject->get(state, namespaceIdentifier);
    if (!master.isObject()) {
      return returnResult;
    }
    function = master.toObject(state)->get(state, funcIdentifier);
  }
  CallData callData;
  CallType callType = getCallData(function, callData);
  NakedPtr<Exception> returnedException;
  JSValue ret = call(state, function, callType, callData, globalObject, obj, returnedException);
  globalObject->vm().drainMicrotasks();

  if (returnedException) {
    ReportException(globalObject, returnedException.get(), instanceId.c_str(), func.c_str());
    return returnResult;
  }
  convertJSArrayToWeexJSResult(state, ret, returnResult.get());
  return returnResult;
}

void WeexRuntimeJSC::exeJSWithCallback(const std::string &instanceId,
                                       const std::string &nameSpace,
                                       const std::string &func,
                                       std::vector<VALUE_WITH_TYPE *> &params,
                                       long callback_id) {
  auto result = exeJSWithResult(instanceId, nameSpace, func, params);
  script_bridge()->core_side()->OnReceivedResult(callback_id, result);
}

std::unique_ptr<WeexJSResult> WeexRuntimeJSC::exeJSOnInstance(const std::string &instanceId,
                                                              const char *script,
                                                              const int script_size,
                                                              unsigned int engine_type) {
  LOGD("test-> [runtime] beofore exeJSOnInstance");
  std::unique_ptr<WeexJSResult> returnResult;
  returnResult = std::make_unique<WeexJSResult>();
  JSGlobalObject *globalObject =
      find_weex_js_instance_context<WeexGlobalObject>(instanceId);
  if (globalObject == nullptr) {
    globalObject = weex_js_global_context<WeexGlobalObject>();
  }
  VM &vm = globalObject->vm();
  JSLockHolder locker(&vm);

  SourceOrigin sourceOrigin(String::fromUTF8("(weex)"));
  NakedPtr<Exception> evaluationException;
  JSValue returnValue = evaluate(globalObject->globalExec(),
                                 makeSource(String::fromUTF8(script),
                                            sourceOrigin,
                                            "execjs on instance context"), JSValue(),
                                 evaluationException);
  globalObject->vm().drainMicrotasks();
  if (evaluationException) {
    // std::string exceptionInfo = exceptionTostd::string(globalObject, evaluationException.get()->value());
    // LOGE("EXECJSONINSTANCE exception:%s", exceptionInfo.utf8().data());
    ReportException(globalObject,
                    evaluationException.get(),
                    instanceId.c_str(),
                    "execJSOnInstance");
    return nullptr;
  }
  // WTF::std::string str = returnValue.toWTFstd::string(globalObject->globalExec());
  const char *data = returnValue.toWTFString(globalObject->globalExec()).utf8().data();
  returnResult->length = strlen(data);
  char *buf = new char[returnResult->length + 1];
  strcpy(buf, data);
  returnResult->data.reset(buf);
  LOGD("test-> [runtime] end exeJSOnInstance");
  return returnResult;
}

int WeexRuntimeJSC::destroyInstance(const std::string &instanceId) {

  // LOGE("IPCJSMsg::DESTORYINSTANCE");
  auto *globalObject =
      find_weex_js_instance_context<WeexGlobalObject>(instanceId);
  if (globalObject == nullptr) {
    return static_cast<int32_t>(true);
  }
  globalObject->destroyTimer();
  // LOGE("Destroy Instance map 11 length:%d", weex_context_holder()->m_jsGlobalObjectMap.size());
  weex_context_holder()->erase(instanceId);
  // LOGE("Destroy Instance map 22 length:%d", weex_context_holder()->m_jsGlobalObjectMap.size());
  // release JSGlobalContextRelease
  return static_cast<int32_t>(true);
}

int WeexRuntimeJSC::updateGlobalConfig(const std::string &config) {
  JSGlobalObject *globalObject = weex_js_global_context<WeexGlobalObject>();
  VM &vm = globalObject->vm();
  JSLockHolder locker(&vm);
  //FIXME, Heron
//    if (weexLiteAppObjectHolder.get() != nullptr) {
//        VM & vm_global = *weexLiteAppObjectHolder->js_engine_vm_.get();
//        JSLockHolder locker_global(&vm_global);
//    }
//  doUpdateGlobalSwitchConfig(config.c_str());
  return static_cast<int32_t>(true);
}

int WeexRuntimeJSC::UpdateInitFrameworkParams(const std::string &key, const std::string &value,
                                              const std::string &desc) {
  JSGlobalObject *globalObject = weex_js_global_context<WeexGlobalObject>();
  VM &vm = globalObject->vm();
  JSLockHolder locker(&vm);
  if (weex_context_holder()->global_weex_context() != nullptr) {
    weex_context_holder()->global_weex_context()->updateInitFrameworkParams(key, value);
  }
  return static_cast<int32_t>(true);
}

int WeexRuntimeJSC::createInstance(const std::string &instanceId,
                                   const std::string &func,
                                   const char *script,
                                   const int script_size,
                                   const std::string &opts,
                                   const std::string &initData,
                                   const std::string &extendsApi,
                                   std::vector<std::pair<std::string, std::string>> params,
                                   unsigned int engine_type) {
  LOG_TLOG("jsEngine", "id --> %s CreateInstance start", instanceId.c_str());
  JSGlobalObject
      *impl_globalObject = weex_js_global_context<WeexGlobalObject>();
  JSGlobalObject *globalObject;
  if (instanceId.empty()) {
    globalObject = impl_globalObject;
  } else {
    auto
        *temp_js_object =
        find_weex_js_instance_context<WeexGlobalObject>(instanceId);
    if (temp_js_object == nullptr) {
      WeexContext *weexContext = weex_context_holder()->cloneWeexObject(
          instanceId,
          true,
          false);
      temp_js_object = js_instance_context<WeexGlobalObject>(weexContext);
      if (temp_js_object == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR,
                            "DYYDEBUG", "temp_js_object context is null");
      }

      VM &vm = temp_js_object->vm();
      JSLockHolder locker(&vm);
      temp_js_object->addExtraOptions(params);
      temp_js_object->SetScriptBridge(script_bridge());
      // --------------------------------------------------

      // use impl global object run createInstanceContext
      // --------------------------------------------------
      // RAx or vue
      JSGlobalObject *globalObject_local = impl_globalObject;
      PropertyName
          createInstanceContextProperty(Identifier::fromString(&vm, "createInstanceContext"));
      ExecState *state = globalObject_local->globalExec();
      JSValue createInstanceContextFunction =
          globalObject_local->get(state, createInstanceContextProperty);
      MarkedArgumentBuffer args;
      args.append(String2JSValue(state, WTF::String::fromUTF8(instanceId.c_str())));
      JSValue optsObject = parseToObject(state, WTF::String::fromUTF8(opts.c_str()));
      args.append(optsObject);
      JSValue initDataObject = parseToObject(state, WTF::String::fromUTF8(initData.c_str()));
      args.append(initDataObject);
      // args.append(std::string2JSValue(state, ""));
      CallData callData;
      CallType callType = getCallData(createInstanceContextFunction, callData);
      NakedPtr<Exception> returnedException;
      JSValue ret = call(state, createInstanceContextFunction, callType, callData,
                         globalObject_local, args, returnedException);
      if (returnedException) {
        // ReportException(globalObject, returnedException.get(), nullptr, "");
        String exceptionInfo = exceptionToString(globalObject_local, returnedException->value());
        LOGE("getJSGlobalObject returnedException %s", exceptionInfo.utf8().data());
      }
      // --------------------------------------------------

      // std::string str = getArgumentAsstd::string(state, ret);
      //(ret.toWTFstd::string(state));

      // use it to set Vue prototype to instance context
      JSObject *object = ret.toObject(state, temp_js_object);
      JSObjectRef ref = toRef(object);
      JSGlobalContextRef globalContextRef = toGlobalRef(state);
      JSGlobalContextRef instanceContextRef = toGlobalRef(temp_js_object->globalExec());
      auto instanceGlobalObject = JSContextGetGlobalObject(instanceContextRef);
      auto pArray = JSObjectCopyPropertyNames(globalContextRef, ref);
      size_t keyCount = JSPropertyNameArrayGetCount(pArray);
      for (size_t i = 0; i < keyCount; ++i) {
        auto propertyName_ = JSPropertyNameArrayGetNameAtIndex(pArray, i);
        if (propertyName_ == nullptr) {
          LOG_TLOG("jsEngine",
                   "id --> %s CreateInstance's propertyName_ is null",
                   instanceId.c_str());
          continue;
        }

        auto propertyValue_ = JSObjectGetProperty(globalContextRef, ref, propertyName_, nullptr);
        if (propertyValue_ == nullptr) {
          LOG_TLOG("jsEngine",
                   "id --> %s CreateInstance's propertyValue_ is null",
                   instanceId.c_str());
          continue;
        }

        JSObjectSetProperty(instanceContextRef,
                            instanceGlobalObject,
                            propertyName_,
                            propertyValue_,
                            0,
                            nullptr);
      }

      JSValueRef vueRef =
          JSObjectGetProperty(globalContextRef, ref, JSStringCreateWithUTF8CString("Vue"), nullptr);
      if (vueRef != nullptr) {
        JSObjectRef vueObject = JSValueToObject(globalContextRef, vueRef, nullptr);
        if (vueObject != nullptr) {

          JSObjectSetPrototype(instanceContextRef, vueObject,
                               JSObjectGetPrototype(instanceContextRef,
                                                    JSContextGetGlobalObject(instanceContextRef)));
        }
      }
      //-------------------------------------------------

//            temp_js_object->resetPrototype(vm, ret);
      weex_context_holder()->put(instanceId, weexContext);
//            LOGE("create Instance instanceId.c_str() %s",instanceId.c_str());
      // -----------------------------------------
      // ExecState* exec =temp_js_object->globalExec();
      // JSLockHolder temp_locker(exec);
      // VM& temp_vm = exec->vm();
      // gcProtect(exec->vmEntryGlobalObject());
      // temp_vm.ref();
      // ------------------------------------------
    }
    globalObject = temp_js_object;
  }

  VM &vm = globalObject->vm();
  JSLockHolder locker(&vm);
  weex::base::TimeCalculator
      timeCalculator(weex::base::TaskPlatform::JSS_ENGINE, "weex run raxApi", instanceId);
  timeCalculator.taskStart();
  // if extend api is not null should exec before createInstanceContext, such as rax-api
  if (!extendsApi.empty() && extendsApi.length() > 0) {
    if (!ExecuteJavaScript(globalObject,
                           WTF::String::fromUTF8(extendsApi.c_str()),
                           ("weex run raxApi"),
                           true,
                           "runRaxApi",
                           instanceId.c_str())) {
      LOG_TLOG("jsEngine", "id --> %s CreateInstance's weex run raxApi failed", instanceId.c_str());
      return static_cast<int32_t>(false);
    }
  }
  timeCalculator.taskEnd();

  if (!ExecuteJavaScript(globalObject,
                         WTF::String::fromUTF8(script),
                         ("weex createInstanceContext"),
                         true,
                         "createInstanceContext",
                         instanceId.c_str())) {
    LOGE("createInstanceContext and ExecuteJavaScript Error script size is %d", script_size);
    LOG_TLOG("jsEngine",
             "id --> %s CreateInstance's createInstanceContext failed",
             instanceId.c_str());
    return static_cast<int32_t>(false);
  }
  return static_cast<int32_t>(true);
}

int WeexRuntimeJSC::_initFramework(const std::string &source) {
  JSLockHolder locker(engine_vm<VM>());
  auto globalObject = weex_js_global_context<WeexGlobalObject>();
  globalObject->SetScriptBridge(script_bridge());
  if (!ExecuteJavaScript(globalObject,
                         WTF::String::fromUTF8(source.c_str()),
                         "(weex framework)",
                         true,
                         "initFramework",
                         "")) {
    return false;
  }

  setJSFVersion(globalObject);
  return true;
}

static void _getArgListFromJSParams(MarkedArgumentBuffer *obj, ExecState *state,
                                    const std::vector<VALUE_WITH_TYPE *> &params) {
  //delete
  String msg = "exejs Args ";
  weex::base::TimeCalculator
      timeCalculator(weex::base::TaskPlatform::JSS_ENGINE, "exejs Args", "exec js");
  timeCalculator.taskStart();

  for (unsigned int i = 0; i < params.size(); i++) {
    VALUE_WITH_TYPE *paramsObject = params[i];
    switch (paramsObject->type) {
      case ParamsType::DOUBLE:obj->append(jsNumber(paramsObject->value.doubleValue));
//                msg.append(":");
//                msg.append(std::to_string(paramsObject->value.doubleValue).c_str());
        break;
      case ParamsType::STRING: {
        WeexString *ipcstr = paramsObject->value.string;
        const String &string2String = weexString2String(ipcstr);
        obj->append(jString2JSValue(state, ipcstr->content, ipcstr->length));

        msg.append(":");
        msg.append(string2String.utf8().data());
      }
        break;
      case ParamsType::JSONSTRING: {
        const WeexString *ipcstr = paramsObject->value.string;
        const String &string = weexString2String(ipcstr);
        String str = jString2String(ipcstr->content, ipcstr->length);
        JSValue o = parseToObject(state, str);
        obj->append(o);

        msg.append(":");
        msg.append(str.utf8().data());
      }
        break;
      case ParamsType::BYTEARRAY: {
        const WeexByteArray *array = paramsObject->value.byteArray;
        JSValue o = wson::toJSValue(state, (void *) array->content, array->length);

        obj->append(o);

        msg.append(":");
        msg.append(JSONStringify(state, o, 0).utf8().data());
      }
        break;
      default:obj->append(jsUndefined());
        break;
    }
  }
  timeCalculator.taskEnd();
  timeCalculator.setArgs(msg.utf8().data());
}












