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

#include "base/string_util.h"
#include "base/log_defines.h"
#include "weex_runtime_qjs.h"
#include "weex_context_qjs.h"
#include "base/string_util.h"
#include "core/bridge/script_bridge.h"
#if OS_ANDROID
#include "wson/wson_parser.h"
#include "android/utils/wson_qjs.h"
#endif
static void ReportException(JSContext *context,
                            const std::string &funcName,
                            const std::string &page_id, ScriptBridge *bridge) {
  const JSValue &exception = JS_GetException(context);
  const char *exception_info = JS_ToCString(context, exception);
  JSValue name = JS_GetPropertyStr(context, exception, "name");
  const char *error_name = JS_ToCString(context, name);
  JSValue stack = JS_GetPropertyStr(context, exception, "stack");
  const char *stack_str = JS_ToCString(context, stack);

  std::ostringstream exception_msg;
  exception_msg << "JS Exception : " << (exception_info == nullptr ? "null" : exception_info)
                << std::endl;
  if (error_name != nullptr) {
    exception_msg << "exception name : " << error_name << std::endl;
  }

  if (stack_str != nullptr) {
    exception_msg << "exception stack : " << stack_str << std::endl;
  }
  bridge->core_side()->ReportException(page_id.c_str(),
                                       funcName.c_str(),
                                       exception_msg.str().c_str());

  JS_FreeValue(context, exception);
  JS_FreeValue(context, name);
  JS_FreeValue(context, stack);
}
static void finish_quickjs_PendingJob(JSRuntime *rt) {
  int ret = 0;
  JSContext *x;
  do {
    ret = JS_ExecutePendingJob(rt, &x);
  } while (ret > 0);
}

static inline void convertJSValueToWeexJSResult(JSContext *ctx,
                                                JSValue &ret,
                                                WeexJSResult *jsResult) {
#if OS_ANDROID
    wson_buffer *buffer = nullptr;
    buffer = toWsonBuffer(ctx, ret);
    jsResult->length = buffer->position;
    char *buf = new char[jsResult->length + 1];
    memcpy(buf, ((char *) buffer->data), jsResult->length);
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

WeexRuntimeQJS::WeexRuntimeQJS(WeexCore::ScriptBridge *script_bridge,
                               bool isMultiProgress) : WeexRuntime(script_bridge, isMultiProgress) {
  set_js_runtime(JS_NewRuntime());
  set_weex_context_holder(std::make_unique<WeexContextHolder>(engine_vm<JSRuntime>(),
                                                              new WeexContextQJS(script_bridge),
                                                              isMultiProgress));

  set_engine_type(JSEngineType::ENGINE_QJS);
}
int WeexRuntimeQJS::initFramework(const std::string &script,
                                  std::vector<std::pair<std::string, std::string>> params) {
  weex_context_holder()->initFromParams(params, false);
  auto *globalContext = weex_js_global_context<JSContext>();
  const JSValue &value =
      JS_Eval(globalContext, script.c_str(), script.length(),
              "jsFramework", JS_EVAL_TYPE_GLOBAL);
  finish_quickjs_PendingJob(engine_vm<JSRuntime>());
  if (JS_IsException(value)) {
    ReportException(globalContext, "initFramework", "", script_bridge());
    return 0;
  } else {
    return 1;
  }
}
int WeexRuntimeQJS::initAppFramework(const std::string &instanceId,
                                     const std::string &appFramework,
                                     std::vector<std::pair<std::string, std::string>> params) {
  return 0;
}
int WeexRuntimeQJS::createAppContext(const std::string &instanceId, const std::string &jsBundle) {
  return 0;
}
int WeexRuntimeQJS::callJSOnAppContext(const std::string &instanceId,
                                       const std::string &func,
                                       std::vector<VALUE_WITH_TYPE *> &params) {
  return 0;
}
int WeexRuntimeQJS::destroyAppContext(const std::string &instanceId) {
  return 0;
}
int WeexRuntimeQJS::exeJsService(const std::string &source) {
  auto *globalContext = weex_js_global_context<JSContext>();
  auto ret = JS_Eval(globalContext, source.c_str(), source.length(),
                     "exeJsService", JS_EVAL_TYPE_GLOBAL);

  finish_quickjs_PendingJob(engine_vm<JSRuntime>());

  int retValue = 0;
  if (JS_IsException(ret)) {
    ReportException(globalContext, "exeJsService", "", script_bridge());
    retValue = 0;
  } else {
    retValue = 1;
  }
  JS_FreeValue(globalContext, ret);
  return retValue;
}
int WeexRuntimeQJS::exeCTimeCallback(const std::string &source) {
  return 0;
}
int WeexRuntimeQJS::exeJS(const std::string &instanceId,
                          const std::string &nameSpace,
                          const std::string &func,
                          const std::vector<VALUE_WITH_TYPE *> &params) {
  std::string newFunc = func;
  JSContext *thisContext = nullptr;

  if (std::strcmp("callJS", func.c_str()) == 0 ||
      std::strcmp("__WEEX_CALL_JAVASCRIPT__", func.c_str()) == 0) {
    thisContext = find_weex_js_instance_context<JSContext>(instanceId);
    if (thisContext == nullptr) {
      thisContext = weex_js_global_context<JSContext>();
    } else {
      newFunc = "__WEEX_CALL_JAVASCRIPT__";
    }
  } else {
    thisContext = weex_js_global_context<JSContext>();;
  }

  auto thisObject = JS_GetGlobalObject(thisContext);

  const int size = (int) params.size();
  JSValue jsValueArray[size];
  for (size_t i = 0; i < size; i++) {
    VALUE_WITH_TYPE *paramsObject = params[i];
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
      case ParamsType::BYTEARRAYJSONSTRING:{
          const WeexByteArray *array = paramsObject->value.byteArray;
          const char *string = array->content;
          jsValueArray[i] = JS_ParseJSON(thisContext, string, array->length, "t");
      }
        break;
      case ParamsType::BYTEARRAY: {
#if OS_ANDROID
        const WeexByteArray *array = paramsObject->value.byteArray;
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
    ReportException(thisContext, "exejs", instanceId, script_bridge());
  }
  finish_quickjs_PendingJob(engine_vm<JSRuntime>());

  for (JSValue arg : jsValueArray) {
    JS_FreeValue(thisContext, arg);
  }

  JS_FreeValue(thisContext, ret);
  return 1;
}
std::unique_ptr<WeexJSResult> WeexRuntimeQJS::exeJSWithResult(const std::string &instanceId,
                                                              const std::string &nameSpace,
                                                              const std::string &func,
                                                              std::vector<VALUE_WITH_TYPE *> &params) {
  std::unique_ptr<WeexJSResult> returnResult(new WeexJSResult);
  std::string newFunc = func;
  JSContext *thisContext = nullptr;

  if (std::strcmp("callJS", func.c_str()) == 0 ||
      std::strcmp("__WEEX_CALL_JAVASCRIPT__", func.c_str()) == 0) {
    thisContext = find_weex_js_instance_context<JSContext>(instanceId);
    if (thisContext == nullptr) {
      thisContext = weex_js_global_context<JSContext>();
    } else {
      newFunc = "__WEEX_CALL_JAVASCRIPT__";
    }
  } else {
    thisContext = weex_js_global_context<JSContext>();
  }

  auto thisObject = JS_GetGlobalObject(thisContext);

  const int size = (int) params.size();
  JSValue js_args[size];

  for (size_t i = 0; i < size; i++) {
    VALUE_WITH_TYPE *paramsObject = params[i];
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
      case ParamsType::BYTEARRAYJSONSTRING:{
        const WeexByteArray *array = paramsObject->value.byteArray;
        const char *string = array->content;
        js_args[i] = JS_ParseJSON(thisContext, string, array->length, "t");
      }
            break;
      case ParamsType::BYTEARRAY: {
#if OS_ANDROID
        const WeexByteArray *array = paramsObject->value.byteArray;

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
  finish_quickjs_PendingJob(engine_vm<JSRuntime>());

  if (JS_IsException(funcVal)) {
    ReportException(thisContext, "exeJSWithResult", instanceId, script_bridge());
  } else {
    convertJSValueToWeexJSResult(thisContext, ret, returnResult.get());
  }
  for (JSValue arg: js_args) {
    JS_FreeValue(thisContext, arg);
  }

  JS_FreeValue(thisContext, thisObject);
  JS_FreeValue(thisContext, funcVal);
  JS_FreeValue(thisContext, ret);
  return returnResult;
}
void WeexRuntimeQJS::exeJSWithCallback(const std::string &instanceId,
                                       const std::string &nameSpace,
                                       const std::string &func,
                                       std::vector<VALUE_WITH_TYPE *> &params,
                                       long callback_id) {
  auto result = exeJSWithResult(instanceId, nameSpace, func, params);
  script_bridge()->core_side()->OnReceivedResult(callback_id, result);
}
int WeexRuntimeQJS::createInstance(const std::string &instanceId,
                                   const std::string &func,
                                   const char *script,
                                   const int script_size,
                                   const std::string &opts,
                                   const std::string &initData,
                                   const std::string &extendsApi,
                                   std::vector<std::pair<std::string, std::string>> params,
                                   unsigned int engine_type) {
  auto *globalContext = weex_js_global_context<JSContext>();
  JSContext *thisContext;
  if (instanceId.empty()) {
    thisContext = globalContext;
  } else {

    WeexContext *weexContext = weex_context_holder()->cloneWeexObject(
        instanceId,
        true,
        false);

    weexContext->addExtraOptions(params);

    thisContext = js_instance_context<JSContext>(weexContext);
    JSValue createInstanceContextFunc =
        JS_GetProperty(globalContext, JS_GetGlobalObject(globalContext),
                       JS_NewAtom(globalContext, "createInstanceContext"));
    JSValue args[3];
    args[0] = JS_NewString(globalContext, instanceId.c_str());
    args[1] = JS_ParseJSON(globalContext, opts.c_str(), opts.length(), "");
    args[2] = JS_ParseJSON(globalContext, initData.c_str(), initData.length(), "");

    JSValue ret = JS_Call(globalContext, createInstanceContextFunc,
                          JS_GetGlobalObject(globalContext), 3, args);
    finish_quickjs_PendingJob(engine_vm<JSRuntime>());

    for (JSValue arg : args) {
      JS_FreeValue(globalContext, arg);
    }

    JS_FreeValue(globalContext, createInstanceContextFunc);

    if (JS_IsException(ret)) {
      const JSValue &exception = JS_GetException(globalContext);
      const char *string = JS_ToCString(globalContext, exception);
      JS_FreeValue(globalContext, ret);
      LOGE("createInstance jscall get error ret: %s", string);
      return 0;
    }

    JSPropertyEnum *tab_atom;
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
      const char *name = JS_AtomToCString(thisContext, atom);
      auto newAtom = JS_NewAtom(thisContext, name);
      JS_SetProperty(thisContext, thisObject, newAtom, propertyValue);
      if (strcmp(name, "Vue") == 0) {
        JSValue val = JS_GetPrototype(thisContext, thisObject);
        JS_SetPrototype(thisContext, propertyValue,
                        val);
      }
    }
    weex_context_holder()->put(instanceId, weexContext);
    JS_FreeValue(thisContext, thisObject);
  }

  if (!extendsApi.empty()) {
    auto ret = JS_Eval(thisContext, extendsApi.c_str(), extendsApi.length(),
                       "extendsApi", JS_EVAL_TYPE_GLOBAL);
    finish_quickjs_PendingJob(engine_vm<JSRuntime>());
    if (JS_IsException(ret)) {
      const JSValue &exception = JS_GetException(thisContext);
      const char *string = JS_ToCString(thisContext, exception);
      LOGE("createInstance eval extendsApi get error ret: %s", string);
      JS_FreeValue(thisContext, ret);
      return 0;
    }

    JS_FreeValue(thisContext, ret);
  }
  JSValue createInstanceRet;
  if (engine_type == ENGINE_QJS_BIN) {
#if OS_ANDROID
    createInstanceRet =
        JS_EvalBinary(thisContext, reinterpret_cast<const uint8_t *>(script), script_size);
#elif OS_IOS
    createInstanceRet =
        JS_EvalBinary(thisContext, reinterpret_cast<const uint8_t *>(script), script_size, 0);
#endif
  } else {
    createInstanceRet = JS_Eval(thisContext, script, script_size,
                                "createInstance", JS_EVAL_TYPE_GLOBAL);
  }

  int retValue = 0;
  if (JS_IsException(createInstanceRet)) {
    ReportException(thisContext, "createInstance", instanceId, script_bridge());
    retValue = 0;
  } else {
    finish_quickjs_PendingJob(engine_vm<JSRuntime>());
    retValue = 1;
  }

  JS_FreeValue(thisContext, createInstanceRet);

  return retValue;
}
std::unique_ptr<WeexJSResult> WeexRuntimeQJS::exeJSOnInstance(const std::string &instanceId,
                                                              const char *script,
                                                              const int script_size,
                                                              unsigned int engine_type) {
  std::unique_ptr<WeexJSResult> returnResult(new WeexJSResult);
  auto
      *engine_context = find_weex_js_instance_context<JSContext>(instanceId);

  if (engine_context == nullptr) {
    engine_context = weex_js_global_context<JSContext>();
  }

  JSValue ret;
  if (engine_type == ENGINE_QJS_BIN) {
#if OS_ANDROID
    ret = JS_EvalBinary(engine_context, reinterpret_cast<const uint8_t *>(script), script_size);
#elif OS_IOS
      ret = JS_EvalBinary(engine_context, reinterpret_cast<const uint8_t *>(script), script_size, 0);
#endif
  } else {
    ret = JS_Eval(engine_context, script, script_size,
                  "exeJsOnInstance", JS_EVAL_TYPE_GLOBAL);
  }

  finish_quickjs_PendingJob(engine_vm<JSRuntime>());

  const char *data = JS_ToCString(engine_context, ret);
  returnResult->length = strlen(data);
  char *buf = new char[returnResult->length + 1];
  strcpy(buf, data);
  returnResult->data.reset(buf);

  if (JS_IsException(ret)) {
    ReportException(engine_context, "execJsOnInstance", instanceId, script_bridge());
  }

  JS_FreeValue(engine_context, ret);
  return returnResult;
}
int WeexRuntimeQJS::destroyInstance(const std::string &instanceId) {
  void *context = weex_context_holder()->find_js_context(instanceId);
  if (context == nullptr) {
    return 1;
  }

  weex_context_holder()->erase(instanceId);

  return 1;
}
int WeexRuntimeQJS::updateGlobalConfig(const std::string &config) {
//  doUpdateGlobalSwitchConfig(config.c_str());
  return 1;
}
int WeexRuntimeQJS::UpdateInitFrameworkParams(const std::string &key,
                                              const std::string &value,
                                              const std::string &desc) {
  if (weex_context_holder()->global_weex_context() != nullptr) {
    weex_context_holder()->global_weex_context()->updateInitFrameworkParams(key, value);
  }

  return 1;
}

std::unique_ptr<WeexJSResult> WeexRuntimeQJS::exeJSOnAppWithResult(const std::string &instanceId,
                                                                   const std::string &jsBundle) {
  return std::unique_ptr<WeexJSResult>();
}
uint8_t *WeexRuntimeQJS::CompileQuickJSByteCode(const std::string &script, int *out_binary_size) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);
  uint8_t *ret_buf;
  int eval_flags = JS_EVAL_FLAG_COMPILE_ONLY | JS_EVAL_TYPE_GLOBAL;
  JSValue obj = JS_Eval(ctx, script.c_str(), script.length(), "js_file_name", eval_flags);
  if (JS_IsException(obj)) {
    JSValue exception = JS_GetException(ctx);
    const char *exception_str = JS_ToCString(ctx, exception);
    JS_FreeValue(ctx, exception);
    JS_FreeCString(ctx, exception_str);
    JS_FreeValue(ctx, obj);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return nullptr;
  }
  size_t out_buf_len;
  ret_buf = JS_WriteObject(ctx, &out_buf_len, obj, JS_WRITE_OBJ_BYTECODE);
  if (out_binary_size) {
    *out_binary_size = out_buf_len;
  }
  JS_FreeValue(ctx, obj);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return ret_buf;
}
