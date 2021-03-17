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

#include "script_side_in_qjs.h"

#include "android/bridge/script/qjs/qjs_binding.h"
#include "base/string_util.h"
#include "android/utils/wson_qjs.h"
#include "core/render/manager/render_manager.h"

namespace WeexCore {
namespace bridge {
namespace script {

#define countof(x) (sizeof(x) / sizeof((x)[0]))

ScriptSideInQJS::ScriptSideInQJS() {
  js_engine_vm_ = JS_NewRuntime();
  global_context_ = CreateContext(js_engine_vm_);
}

void ScriptSideInQJS::ConvertJSValueToWeexJSResult(JSContext* ctx,
                                                   JSValue &ret,
                                                   WeexJSResult* jsResult) {
#if OS_ANDROID
  wson_buffer* buffer = nullptr;
  buffer = toWsonBuffer(ctx, ret);
  jsResult->length = buffer->position;
  char* buf = new char[jsResult->length + 1];
  memcpy(buf, ((char*) buffer->data), jsResult->length);
  buf[jsResult->length] = '\0';
  jsResult->data.reset(buf);
  wson_buffer_free(buffer);
#else
  size_t length = 0;
    const char *ret_string;
    if (JS_IsString(ret))
    {
      ret_string = JS_ToCStringLen(ctx, &length, ret);
    }
    else if (JS_IsObject(ret))
    {
      JSValue stringify = JS_JSONStringify(ctx, ret, JS_UNDEFINED, JS_NewInt32(ctx, 0));
      ret_string = JS_ToCStringLen(ctx, &length, stringify);
      JS_FreeValue(ctx, stringify);
    }
    char *buf = new char[length + 1];
    memcpy(buf, ret_string, length);
    buf[length] = '\0';
    jsResult->length = (int) length;
    jsResult->data.reset(buf);
    //JS_FreeCString(ctx, ret_string);
#endif
}

void ScriptSideInQJS::FinishQJSPendingJob(JSRuntime* rt) {
  int ret = 0;
  JSContext* x;
  do {
    ret = JS_ExecutePendingJob(rt, &x);
  } while (ret > 0);
}

void ScriptSideInQJS::InitGlobalContextFunctions() {
  static const JSCFunctionListEntry js_fib_funcs[] = {
      JS_CFUNC_DEF("callNative", 3, js_CallNative),
      JS_CFUNC_DEF("callNativeModule", 5, js_CallNativeModule),
      JS_CFUNC_DEF("callNativeComponent", 5, js_CallNativeComponent),
      JS_CFUNC_DEF("callAddElement", 5, js_CallAddElement),
      JS_CFUNC_DEF("setTimeoutNative", 2, js_SetTimeoutNative),
      JS_CFUNC_DEF("nativeLog", 5, js_NativeLog),
      JS_CFUNC_DEF("notifyTrimMemory", 0, js_NotifyTrimMemory),
      JS_CFUNC_DEF("markupState", 0, js_MarkupState),
      JS_CFUNC_DEF("atob", 1, js_Atob),
      JS_CFUNC_DEF("btoa", 1, js_Btoa),
      JS_CFUNC_DEF("callCreateBody", 3, js_CallCreateBody),
      JS_CFUNC_DEF("callUpdateFinish", 3, js_CallUpdateFinish),
      JS_CFUNC_DEF("callCreateFinish", 3, js_CallCreateFinish),
      JS_CFUNC_DEF("callRefreshFinish", 3, js_CallRefreshFinish),
      JS_CFUNC_DEF("callUpdateAttrs", 4, js_CallUpdateAttrs),
      JS_CFUNC_DEF("callUpdateStyle", 4, js_CallUpdateStyle),
      JS_CFUNC_DEF("callRemoveElement", 5, js_CallRemoveElement),
      JS_CFUNC_DEF("callMoveElement", 5, js_CallMoveElement),
      JS_CFUNC_DEF("callAddEvent", 4, js_CallAddEvent),
      JS_CFUNC_DEF("callRemoveEvent", 4, js_CallRemoveEvent),
//      JS_CFUNC_DEF("callGCanvasLinkNative", 3, js_GCanvasLinkNative),
      JS_CFUNC_DEF("setIntervalWeex", 3, js_SetIntervalWeex),
      JS_CFUNC_DEF("clearIntervalWeex", 1, js_ClearIntervalWeex),
//      JS_CFUNC_DEF("callT3DLinkNative", 2, js_T3DLinkNative),
//      JS_CFUNC_DEF("__updateComponentData", 3, js_UpdateComponentData),
  };

  JS_SetPropertyFunctionList(global_context_, JS_GetGlobalObject(global_context_), js_fib_funcs,
                             countof(js_fib_funcs));
  BindConsoleLog(global_context_);
}

void ScriptSideInQJS::InitInstanceContextFunctions(JSContext* context) {
  static const JSCFunctionListEntry js_fib_funcs[] = {
      JS_CFUNC_DEF("nativeLog", 5, js_NativeLog),
      JS_CFUNC_DEF("atob", 1, js_Atob),
      JS_CFUNC_DEF("btoa", 1, js_Btoa),
//      JS_CFUNC_DEF("callGCanvasLinkNative", 3, js_GCanvasLinkNative),
      JS_CFUNC_DEF("setIntervalWeex", 3, js_SetIntervalWeex),
      JS_CFUNC_DEF("clearIntervalWeex", 1, js_ClearIntervalWeex),
//      JS_CFUNC_DEF("callT3DLinkNative", 2, js_T3DLinkNative),
//      JS_CFUNC_DEF("__updateComponentData", 3, js_UpdateComponentData),
//      JS_CFUNC_DEF("setNativeTimeout", 2, js_SetNativeTimeout),
//      JS_CFUNC_DEF("clearNativeTimeout", 1, js_ClearNativeTimeout),
//      JS_CFUNC_DEF("setNativeInterval", 2, js_SetNativeInterval),
//      JS_CFUNC_DEF("clearNativeInterval", 1, js_ClearNativeInterval),
  };
  JS_SetPropertyFunctionList(context, JS_GetGlobalObject(context), js_fib_funcs,
                             countof(js_fib_funcs));
  BindConsoleLog(context);
}

int ScriptSideInQJS::InitFramework(const char* script,
                                   std::vector<std::pair<std::string, std::string>> params) {
  JS_SetContextOpaque(global_context_, this);

  InitWXEnvironment(params, global_context_, true);

  InitGlobalContextFunctions();

  const JSValue &value =
      JS_Eval(global_context_, script, script != nullptr ? strlen(script) : 0,
              "jsFramework", JS_EVAL_TYPE_GLOBAL);
  FinishQJSPendingJob(js_engine_vm_);

  if (JS_IsException(value)) {
    ReportException(global_context_, "initFramework", "", bridge());
    return 0;
  } else {
    return 1;
  }
}

void ScriptSideInQJS::InitWXEnvironment(JSContext* context) {
  const JSValue &jsValue = JS_NewObject(context);
  const JSValue &object = JS_GetGlobalObject(context);

  for (const auto &param : init_framework_args_) {

    const std::string &type = param.first;
    const std::string &value = param.second;

    JS_SetPropertyStr(context, jsValue, type.c_str(),
                      JS_NewString(context, value.c_str()));

    static bool hasSet = false;
    if (!hasSet) {
      if ("debugMode" == type && "true" == value) {
        weex::base::LogImplement::getLog()->setDebugMode(true);
        hasSet = true;
      }
    }
  }
  JS_SetProperty(context, object, JS_NewAtom(context, "WXEnvironment"), jsValue);
}

void ScriptSideInQJS::InitWXEnvironment(std::vector<std::pair<std::string, std::string>>& params,
                                        JSContext* context,
                                        bool is_save) {
  const JSValue &jsValue = JS_NewObject(context);
  const JSValue &object = JS_GetGlobalObject(context);

  for (const auto &param : params) {

    const std::string &type = std::string(param.first);
    const std::string &value = std::string(param.second);

    JS_SetPropertyStr(context, jsValue, type.c_str(),
                      JS_NewString(context, value.c_str()));

    static bool hasSet = false;
    if (!hasSet) {
      if ("debugMode" == type && "true" == value) {
        weex::base::LogImplement::getLog()->setDebugMode(true);
        hasSet = true;
      }
    }

    if (is_save) {
      this->init_framework_args_.emplace_back(type, value);
    }
  }
  JS_SetProperty(context, object, JS_NewAtom(context, "WXEnvironment"), jsValue);
}

JSContext* ScriptSideInQJS::CreateContext(JSRuntime* engine_vm) {
  JSRuntime* rt = engine_vm;
  JSContext* ctx = JS_NewContext(rt);
  JSValue i = JS_GetGlobalObject(ctx);
  JS_SetProperty(ctx, i, JS_NewAtom(ctx, "global"), i);
  JS_SetMaxStackSize(rt, 1024 * 100000);
  return ctx;
}

std::unique_ptr<WeexJSResult> bridge::script::ScriptSideInQJS::ExecJSOnAppWithResult(const char* instanceId,
                                                                                     const char* jsBundle) {
  return std::unique_ptr<WeexJSResult>();
}

int ScriptSideInQJS::ExecJsService(const char* source) {
  auto ret = JS_Eval(global_context_, source, source != nullptr ? strlen(source) : 0,
                     "exeJsService", JS_EVAL_TYPE_GLOBAL);

  FinishQJSPendingJob(js_engine_vm_);

  int retValue = 0;
  if (JS_IsException(ret)) {
    ReportException(global_context_, "exeJsService", "", bridge());
    retValue = 0;
  } else {
    retValue = 1;
  }
  JS_FreeValue(global_context_, ret);
  return retValue;
}

int ScriptSideInQJS::ExecJS(const char *instanceId, const char *nameSpace,
                            const char *func,
                            const std::vector<VALUE_WITH_TYPE *> &params) {
  std::string newFunc = func;
  JSContext* thisContext = nullptr;

  if (std::strcmp("callJS", func) == 0 ||
      std::strcmp("__WEEX_CALL_JAVASCRIPT__", func) == 0) {
    thisContext =
        js_context_map_[instanceId];
    if (thisContext == nullptr) {
      thisContext = global_context_;
    } else {
      newFunc = "__WEEX_CALL_JAVASCRIPT__";
    }
  } else {
    thisContext = global_context_;
  }

  auto thisObject = JS_GetGlobalObject(thisContext);

  const int size = (int) params.size();
  JSValue jsValueArray[size];
  for (size_t i = 0; i < size; i++) {
    VALUE_WITH_TYPE* paramsObject = params[i];
    switch (paramsObject->type) {
      case ParamsType::DOUBLE:
        jsValueArray[i] = JS_NewFloat64(thisContext, paramsObject->value.doubleValue);
        break;
      case ParamsType::STRING: {
        auto string2String = weex::base::weexString2stdString(paramsObject->value.string);
        jsValueArray[i] = JS_NewString(thisContext, string2String.c_str());
      }
        break;
      case ParamsType::JSONSTRING: {
        auto string2String = weex::base::weexString2stdString(paramsObject->value.string);
        jsValueArray[i] = JS_ParseJSON(thisContext, string2String.c_str(),
                                       string2String.length(), "t");
      }
        break;
      case ParamsType::BYTEARRAYJSONSTRING: {
        const WeexByteArray* array = paramsObject->value.byteArray;
        const char* string = array->content;
        jsValueArray[i] = JS_ParseJSON(thisContext, string, array->length, "t");
      }
        break;
      case ParamsType::BYTEARRAY: {
#if OS_ANDROID
        const WeexByteArray* array = paramsObject->value.byteArray;
        wson_parser w(array->content, array->length);
        auto string2String = w.toStringUTF8();
        auto jsvalue = JS_ParseJSON(thisContext, string2String.c_str(),
                                    string2String.length(), "t");
        jsValueArray[i] = jsvalue;
#endif
      }
        break;
        {
          default:jsValueArray[i] = JS_UNDEFINED;
          break;
        }
    }
  }

  auto ret = JS_Call(thisContext,
                     JS_GetProperty(thisContext, thisObject,
                                    JS_NewAtom(thisContext, newFunc.c_str())),
                     thisObject, size, jsValueArray);
  if (JS_IsException(ret)) {
    ReportException(thisContext, "execjs", instanceId, bridge());
  }
  FinishQJSPendingJob(js_engine_vm_);

  for (JSValue arg : jsValueArray) {
    JS_FreeValue(thisContext, arg);
  }

  JS_FreeValue(thisContext, ret);
  return 1;
}

std::unique_ptr<WeexJSResult> ScriptSideInQJS::ExecJSWithResult(const char* instanceId,
                                                                const char* nameSpace,
                                                                const char* func,
                                                                std::vector<
                                                                    VALUE_WITH_TYPE*> &params) {
  std::unique_ptr<WeexJSResult> returnResult(new WeexJSResult);
  std::string newFunc = func;
  JSContext* thisContext = nullptr;

  if (std::strcmp("callJS", func) == 0 ||
      std::strcmp("__WEEX_CALL_JAVASCRIPT__", func) == 0) {
    thisContext =
        js_context_map_[instanceId];
    if (thisContext == nullptr) {
      thisContext = global_context_;
    } else {
      newFunc = "__WEEX_CALL_JAVASCRIPT__";
    }
  } else {
    thisContext = global_context_;
  }

  auto thisObject = JS_GetGlobalObject(thisContext);

  const int size = (int) params.size();
  JSValue js_args[size];

  for (size_t i = 0; i < size; i++) {
    VALUE_WITH_TYPE* paramsObject = params[i];
    switch (paramsObject->type) {
      case ParamsType::DOUBLE:
        js_args[i] = JS_NewInt64(thisContext, paramsObject->value.doubleValue);
        break;
      case ParamsType::STRING: {
        auto string2String = weex::base::weexString2stdString(paramsObject->value.string);
        js_args[i] = JS_NewString(thisContext, string2String.c_str());
      }
        break;
      case ParamsType::JSONSTRING: {
        auto string2String = weex::base::weexString2stdString(paramsObject->value.string);
        auto jsvalue = JS_ParseJSON(thisContext, string2String.c_str(),
                                    string2String.length(), "t");
        js_args[i] = jsvalue;
      }
        break;
      case ParamsType::BYTEARRAYJSONSTRING: {
        const WeexByteArray* array = paramsObject->value.byteArray;
        const char* string = array->content;
        js_args[i] = JS_ParseJSON(thisContext, string, array->length, "t");
      }
      case ParamsType::BYTEARRAY: {
#if OS_ANDROID
        const WeexByteArray* array = paramsObject->value.byteArray;
        wson_parser w(array->content, array->length);
        auto string2String = w.toStringUTF8();
        auto jsvalue = JS_ParseJSON(thisContext, string2String.c_str(),
                                    string2String.length(), "t");
        js_args[i] = jsvalue;
#endif
      }
        break;
      default: {
        js_args[i] = JS_UNDEFINED;
      }
        break;
    }
  }

  JSValue
      funcVal = JS_GetProperty(thisContext, thisObject, JS_NewAtom(thisContext, newFunc.c_str()));
  auto ret = JS_Call(thisContext, funcVal, thisObject, (int) size, js_args);
  FinishQJSPendingJob(js_engine_vm_);

  if (JS_IsException(funcVal)) {
    ReportException(thisContext, "exeJSWithResult", instanceId, bridge());
  } else {
    ConvertJSValueToWeexJSResult(thisContext, ret, returnResult.get());
  }
  for (JSValue arg: js_args) {
    JS_FreeValue(thisContext, arg);
  }

  JS_FreeValue(thisContext, thisObject);
  JS_FreeValue(thisContext, funcVal);
  JS_FreeValue(thisContext, ret);
  return returnResult;
}

std::unique_ptr<WeexJSResult> ScriptSideInQJS::exeJSWithResult(const char* instanceId,
                                                               const char* nameSpace,
                                                               const char* func,
                                                               std::vector<VALUE_WITH_TYPE*> &params) {
  std::unique_ptr<WeexJSResult> returnResult(new WeexJSResult);
  std::string newFunc = func;
  JSContext* thisContext = nullptr;

  if (std::strcmp("callJS", func) == 0 ||
      std::strcmp("__WEEX_CALL_JAVASCRIPT__", func) == 0) {
    thisContext =
        js_context_map_[instanceId];
    if (thisContext == nullptr) {
      thisContext = global_context_;
    } else {
      newFunc = "__WEEX_CALL_JAVASCRIPT__";
    }
  } else {
    thisContext = global_context_;
  }

  auto thisObject = JS_GetGlobalObject(thisContext);

  const int size = (int) params.size();
  JSValue js_args[size];

  for (size_t i = 0; i < size; i++) {
    VALUE_WITH_TYPE* paramsObject = params[i];
    switch (paramsObject->type) {
      case ParamsType::DOUBLE:
        js_args[i] = JS_NewInt64(thisContext, paramsObject->value.doubleValue);
        break;
      case ParamsType::STRING: {
        auto string2String = weex::base::weexString2stdString(paramsObject->value.string);
        js_args[i] = JS_NewString(thisContext, string2String.c_str());
      }
        break;
      case ParamsType::JSONSTRING: {
        auto string2String = weex::base::weexString2stdString(paramsObject->value.string);
        auto jsvalue = JS_ParseJSON(thisContext, string2String.c_str(),
                                    string2String.length(), "t");
        js_args[i] = jsvalue;
      }
        break;
      case ParamsType::BYTEARRAYJSONSTRING: {
        const WeexByteArray* array = paramsObject->value.byteArray;
        const char* string = array->content;
        js_args[i] = JS_ParseJSON(thisContext, string, array->length, "t");
      }
      case ParamsType::BYTEARRAY: {
#if OS_ANDROID
        const WeexByteArray* array = paramsObject->value.byteArray;

        wson_parser w(array->content, array->length);
        auto string2String = w.toStringUTF8();
        auto jsvalue = JS_ParseJSON(thisContext, string2String.c_str(),
                                    string2String.length(), "t");
        js_args[i] = jsvalue;
#endif
      }
        break;
      default: {
        js_args[i] = JS_UNDEFINED;
      }
        break;
    }
  }

  JSValue
      funcVal = JS_GetProperty(thisContext, thisObject, JS_NewAtom(thisContext, newFunc.c_str()));
  auto ret = JS_Call(thisContext, funcVal, thisObject, (int) size, js_args);
  FinishQJSPendingJob(js_engine_vm_);

  if (JS_IsException(funcVal)) {
    ReportException(thisContext, "exeJSWithResult", instanceId, bridge());
  } else {
    ConvertJSValueToWeexJSResult(thisContext, ret, returnResult.get());
  }
  for (JSValue arg: js_args) {
    JS_FreeValue(thisContext, arg);
  }

  JS_FreeValue(thisContext, thisObject);
  JS_FreeValue(thisContext, funcVal);
  JS_FreeValue(thisContext, ret);
  return returnResult;
}

void ScriptSideInQJS::ExecJSWithCallback(const char* instanceId,
                                         const char* nameSpace,
                                         const char* func,
                                         std::vector<VALUE_WITH_TYPE*> &params,
                                         long callback_id) {
  auto result = exeJSWithResult(instanceId, nameSpace, func, params);
  bridge()->core_side()->OnReceivedResult(callback_id, result);
}

int ScriptSideInQJS::CreateInstance(const char *instanceId,
                                    const char *func,
                                    const char *script,
                                    const int script_size,
                                    const char *opts,
                                    const char *initData,
                                    const char *extendsApi,
                                    std::vector<std::pair<std::string, std::string>> params) {
  LOGE("create instance start");
  auto* globalContext = global_context_;
  JSContext* thisContext;
  if (instanceId == nullptr || strlen(instanceId) == 0) {
    thisContext = globalContext;
  } else {
//TODO params ??
    thisContext = CreateContext(js_engine_vm_);
    InitWXEnvironment(thisContext);
    InitInstanceContextFunctions(thisContext);
    JS_SetContextOpaque(thisContext, this);

    JSValue createInstanceContextFunc =
        JS_GetProperty(globalContext, JS_GetGlobalObject(globalContext),
                       JS_NewAtom(globalContext, "createInstanceContext"));
    JSValue args[3];
    args[0] = JS_NewString(globalContext, instanceId);
    args[1] = JS_ParseJSON(globalContext, opts, opts != nullptr ? strlen(opts) : 0, "");
    args[2] = JS_ParseJSON(globalContext, initData, initData != nullptr ? strlen(initData) : 0, "");

    JSValue ret = JS_Call(globalContext, createInstanceContextFunc,
                          JS_GetGlobalObject(globalContext), 3, args);

    for (JSValue arg : args) {
      JS_FreeValue(globalContext, arg);
    }

    JS_FreeValue(globalContext, createInstanceContextFunc);

    if (JS_IsException(ret)) {
      const JSValue &exception = JS_GetException(globalContext);
      const char* string = JS_ToCString(globalContext, exception);
      JS_FreeValue(globalContext, ret);
      LOGE("createInstance jscall get error ret: %s", string);
      return 0;
    }

    JSPropertyEnum* tab_atom;
    uint32_t tab_atom_count;
    if (JS_GetOwnPropertyNames(
        globalContext, &tab_atom, &tab_atom_count, ret,
        JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK)) {
      return 0;
    }

    auto thisObject = JS_GetGlobalObject(thisContext);
    for (size_t i = 0; i < tab_atom_count; i++) {
      auto atom = tab_atom[i].atom;
      auto propertyValue = JS_GetProperty(globalContext, ret, atom);
      const char* name = JS_AtomToCString(thisContext, atom);
      auto newAtom = JS_NewAtom(thisContext, name);
      JS_SetProperty(thisContext, thisObject, newAtom, propertyValue);
      if (strcmp(name, "Vue") == 0) {
        JSValue val = JS_GetPrototype(thisContext, thisObject);
        JS_SetPrototype(thisContext, propertyValue,
                        val);
      }
    }
    js_context_map_[instanceId] = thisContext;
    JS_FreeValue(thisContext, thisObject);
  }

  if (extendsApi != nullptr && strlen(extendsApi) > 0) {
    LOGE("create instance execute extends api in qjs start %d", strlen(extendsApi));
    if (!ExecuteExtendsScript(thisContext, instanceId, extendsApi)) {
      return 0;
    }
  }

  LOGE("create instance execute script in qjs start");
  int retValue = ExecuteScript(thisContext, instanceId, script);
  LOGE("create instance execute script in qjs end");
  WeexCoreManager::Instance()
      ->script_bridge()
      ->core_side()
      ->CallNative(instanceId, "HeartBeat", "HeartBeat");
  return retValue;
}

std::unique_ptr<WeexJSResult> ScriptSideInQJS::ExecJSOnInstance(const char *instanceId,
                                                                const char *script,
                                                                const int script_size,
                                                                int type) {
  std::unique_ptr<WeexJSResult> returnResult(new WeexJSResult);
  auto* engine_context =
      js_context_map_[instanceId];

  if (engine_context == nullptr) {
    engine_context = global_context_;
  }

  JSValue ret;
//  if (engine_type == ENGINE_QJS_BIN) {
//#if OS_ANDROID
//    ret = JS_EvalBinary(engine_context, reinterpret_cast<const uint8_t *>(script), script_size);
//#elif OS_IOS
//    ret = JS_EvalBinary(engine_context, reinterpret_cast<const uint8_t *>(script), script_size, 0);
//#endif
//  } else {
  ret = JS_Eval(engine_context, script, script != nullptr ? strlen(script) : 0,
                "exeJsOnInstance", JS_EVAL_TYPE_GLOBAL);
//  }

  FinishQJSPendingJob(js_engine_vm_);

  const char* data = JS_ToCString(engine_context, ret);
  returnResult->length = data != nullptr ? strlen(data) : 0;
  char* buf = new char[returnResult->length + 1];
  strcpy(buf, data);
  returnResult->data.reset(buf);

  if (JS_IsException(ret)) {
    ReportException(engine_context, "execJsOnInstance", instanceId, bridge());
  }

  JS_FreeValue(engine_context, ret);
  return returnResult;
}

int ScriptSideInQJS::DestroyInstance(const char* instanceId) {
  JSContext* context = js_context_map_[instanceId];
  if (context == nullptr) {
    return 1;
  }

  JS_FreeContext(context);
  js_context_map_.erase(instanceId);

  return 1;
}

int ScriptSideInQJS::UpdateInitFrameworkParams(const std::string &key,
                                               const std::string &value,
                                               const std::string &desc) {

  for (auto param : init_framework_args_) {
    if (key == param.first && value != param.second) {
      param.second = value;
    }
  }

  return 1;
}

void ScriptSideInQJS::SetLogType(const int logLevel, const bool isPerf) {
}

int ScriptSideInQJS::ExecuteExtendsScript(JSContext* ctx,
                                          const char* instance_id,
                                          const char* script) {
  int ret_value = 0;
  JSValue value;
  if (!cached_extends_qjs_byte_code_) {
    value = JS_Eval(ctx, script,
                    script != nullptr ? strlen(script) : 0, "extendsApi", JS_EVAL_FLAG_COMPILE_ONLY);
    if (JS_IsException(value)) {
      ReportException(ctx, "ExecuteExtendsScript_JS_Eval", instance_id, bridge());
    } else {
      size_t ret_buf_len;
      cached_extends_qjs_byte_code_ = JS_WriteObject(ctx, &ret_buf_len, value, JS_WRITE_OBJ_BYTECODE);
      value = JS_EvalFunction(ctx, value);
      if (JS_IsException(value)) {
        ReportException(ctx, "ExecuteExtendsScript_JS_EvalFunction", instance_id, bridge());
      } else {
        FinishQJSPendingJob(js_engine_vm_);
        ret_value = 1;
      }
    }
  } else {
    value =
        JS_EvalBinary(ctx, cached_extends_qjs_byte_code_, script != nullptr ? strlen(script) : 0);
    if (JS_IsException(value)) {
      ReportException(ctx, "ExecuteExtendsScript_JS_EvalBinary", instance_id, bridge());
    } else {
      ret_value = 1;
    }
  }
  JS_FreeValue(ctx, value);
  return ret_value;
}

int ScriptSideInQJS::ExecuteScript(JSContext* ctx,
                                   const char* instance_id,
                                   const char* script) {
  std::string enable_qjs_bin_cache_cfg =
      RenderManager::GetInstance()->getPageArgument(std::string(instance_id),
                                                    "enable_qjs_bin_cache");
  bool enable_qjs_bin_cache = enable_qjs_bin_cache_cfg == "true";
  std::string qjs_bin_cache_key =
      RenderManager::GetInstance()->getPageArgument(std::string(instance_id),
                                                    "qjs_bin_cache_key");
  if (enable_qjs_bin_cache && qjs_bin_cache_key.empty()) {
    enable_qjs_bin_cache = false;
  }
  int ret_value = 0;

  JSValue value;
  if (enable_qjs_bin_cache) {
    uint8_t* cached_data = cached_qjs_byte_code_[qjs_bin_cache_key];
    if (cached_data) {
      LOGE("execute script in qjs eval binary start");
      value =
          JS_EvalBinary(ctx, cached_data, script != nullptr ? strlen(script) : 0);
      if (JS_IsException(value)) {
        ReportException(ctx, "ExecuteScript_JS_EvalBinary", instance_id, bridge());
      } else {
        ret_value = 1;
      }
      LOGE("execute script in qjs eval binary end");
    }
  }

  if (!ret_value) {
    LOGE("execute script in qjs eval start");
    value = JS_Eval(ctx, script,
        script != nullptr ? strlen(script) : 0, "script_file",
                    enable_qjs_bin_cache ? JS_EVAL_FLAG_COMPILE_ONLY : JS_EVAL_TYPE_GLOBAL);
    LOGE("execute script in qjs eval end");

    if (JS_IsException(value)) {
      ReportException(ctx, "ExecuteScript_JS_Eval", instance_id, bridge());
    } else {
      if (enable_qjs_bin_cache) {
        LOGE("execute script in qjs write cache start");
        size_t ret_buf_len;
        uint8_t* ret_buf = JS_WriteObject(ctx, &ret_buf_len, value, JS_WRITE_OBJ_BYTECODE);
        cached_qjs_byte_code_[qjs_bin_cache_key] = ret_buf;
        LOGE("execute script in qjs write cache end");

        LOGE("execute script in qjs eval func start");
        value = JS_EvalFunction(ctx, value);
        LOGE("execute script in qjs eval func end");
      }

      if (enable_qjs_bin_cache && JS_IsException(value)) {
        ReportException(ctx, "ExecuteScript_JS_EvalFunction", instance_id, bridge());
      } else {
        FinishQJSPendingJob(js_engine_vm_);
        ret_value = 1;
      }
    }
  }

  JS_FreeValue(ctx, value);
  return ret_value;
}

void ScriptSideInQJS::JSParams::set_value(JSContext* context, JSValue value) {

#if OS_ANDROID
  if (param_type == PARAMS_TYPE_WSON) {
    wson_buffer_ = toWsonBuffer(context, value);
    value_ = (char*) (wson_buffer_->data);
    length_ = wson_buffer_->position;
  } else {
    ctx_ = context;
    if (!JS_IsString(value)) {
      value = JS_JSONStringify(context, value, JS_UNDEFINED,
                               JS_NewInt32(context, 0));
    }
    value_ = const_cast<char*>(JS_ToCStringLen(context,
                                               &length_,
                                               value));
  }
#else
  ctx_ = context;
    if(!JS_IsString(value)) {
      value = JS_JSONStringify(context, value, JS_UNDEFINED,
                               JS_NewInt32(context, 0));
    }
    value_ = const_cast<char *>(JS_ToCStringLen(context,
                                                &length_,
                                                value));
#endif
}

ScriptSideInQJS::JSParams::~JSParams() {
#if OS_ANDROID
  if (param_type == PARAMS_TYPE_WSON) {
    if (wson_buffer_ != nullptr) {
      wson_buffer_free(wson_buffer_);
    }
  } else {
    if (value_ != nullptr && ctx_ != nullptr) {
      JS_FreeCString(ctx_, value_);
    }
  }
#else
  if (value_ != nullptr && ctx_ != nullptr) {
      JS_FreeCString(ctx_, value_);
    }
#endif
  ctx_ = nullptr;
  value_ = nullptr;
  length_ = 0;
}

ScriptSideInQJS::JSParams::JSParams(JSContext* context, JSValue value, int type) {
  this->param_type = type;
  ctx_ = nullptr;
  value_ = nullptr;
  length_ = 0;
#if OS_ANDROID
  wson_buffer_ = nullptr;
#endif
  set_value(context, value);
}

}  // namespace script
}  // namespace bridge
}  // namespace WeexCore
