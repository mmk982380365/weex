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
// Created by 董亚运 on 2020/8/8.
//

#include "base/string_util.h"
#include "base/utils/log_utils.h"
#include "weex_context_qjs.h"
#include "core/bridge/script_bridge.h"
#include "log_defines.h"

#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"

#ifdef OS_IOS
extern "C"{
#include "WXTimeOutModule.h"
}
#endif

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static std::shared_ptr<WeexCore::ScriptBridge> bridge(JSContext *ctx) {
  if (ctx == nullptr) {
    return nullptr;
  }

  void *Opaque = JS_GetContextOpaque(ctx);
  if (Opaque == nullptr) {
    return nullptr;
  }

  auto *kPContext = static_cast<WeexContext *>(Opaque);
  if (kPContext == nullptr) {
    return nullptr;
  }

  return kPContext->script_bridge();
}

static JSValue js_GCAndSweep(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv);

static JSValue js_CallNative(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv);

static JSValue js_CallNativeModule(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_CallNativeComponent(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

static JSValue js_CallAddElement(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv);

static JSValue js_SetTimeoutNative(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_NativeLog(JSContext *ctx, JSValueConst this_val, int argc,
                            JSValueConst *argv);

static JSValue js_NotifyTrimMemory(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_MarkupState(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv);

static JSValue js_Atob(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv);

static JSValue js_Btoa(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv);

static JSValue js_CallCreateBody(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv);

static JSValue js_CallUpdateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_CallCreateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_CallRefreshFinish(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_CallUpdateAttrs(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_CallUpdateStyle(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_CallRemoveElement(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_CallMoveElement(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_CallAddEvent(JSContext *ctx, JSValueConst this_val, int argc,
                               JSValueConst *argv);

static JSValue js_CallRemoveEvent(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_GCanvasLinkNative(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_SetIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_ClearIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_T3DLinkNative(JSContext *ctx, JSValueConst this_val, int argc,
                                JSValueConst *argv);

static JSValue js_NativeLogContext(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_DisPatchMessage(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_DispatchMessageSync(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

static JSValue js_PostMessage(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv);

static JSValue addNativeTimer(JSContext *ctx, JSValueConst this_val,
                              int argc, JSValueConst *argv, bool repeat);

#ifdef OS_IOS
static JSValue js_SetTimeout(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv);
#endif

static JSValue js_SetNativeTimeout(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_SetNativeInterval(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_ClearNativeTimeout(JSContext *ctx, JSValueConst this_val,
                                     int argc, JSValueConst *argv);

static JSValue js_ClearNativeInterval(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

// For data render
static JSValue js_UpdateComponentData(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

//    Log = 1,
//    Warning = 2,
//    Error = 3,
//    Debug = 4,
//    Info = 5,
static JSValue js_print(JSContext *ctx, JSValueConst this_val, int argc,
                        JSValueConst *argv, int level) {
  if (argc == 0) {
    return JS_TRUE;
  }

  JSValueConst msg = argv[0];
  const char *str = nullptr;

  int tag = JS_VALUE_GET_TAG(msg);
  if (tag == JS_TAG_STRING) {
    str = JS_ToCString(ctx, msg);
  } else if (tag == JS_TAG_OBJECT) {
    JSValue stringify = JS_JSONStringify(ctx, msg, JS_UNDEFINED, JS_NewInt32(ctx, 0));
    if (!JS_IsException(stringify)) {
      str = JS_ToCString(ctx, stringify);
    }
  }

  if (str == nullptr) {
    return JS_FALSE;
  }

  Weex::LogUtil::ConsoleLogPrint((int) level, "jsLog", str);
  return JS_TRUE;
}

static JSValue js_print_log(JSContext *ctx, JSValueConst this_val, int argc,
                            JSValueConst *argv) {
  return js_print(ctx, this_val, argc, argv, 1);
}

static JSValue js_print_info(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  return js_print(ctx, this_val, argc, argv, 5);
}

static JSValue js_print_debug(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  return js_print(ctx, this_val, argc, argv, 4);
}

static JSValue js_print_error(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  return js_print(ctx, this_val, argc, argv, 3);
}
static JSValue js_print_warn(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  return js_print(ctx, this_val, argc, argv, 2);
}

static void bindConsoleLog(JSContext *ctx) {
  JSValue global_obj, console;
  global_obj = JS_GetGlobalObject(ctx);
  console = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, console, "log",
                    JS_NewCFunction(ctx, js_print_log, "log", 1));

  JS_SetPropertyStr(ctx, console, "error",
                    JS_NewCFunction(ctx, js_print_error, "error", 1));

  JS_SetPropertyStr(ctx, console, "info",
                    JS_NewCFunction(ctx, js_print_info, "info", 1));

  JS_SetPropertyStr(ctx, console, "debug",
                    JS_NewCFunction(ctx, js_print_debug, "debug", 1));

  JS_SetPropertyStr(ctx, console, "warn",
                    JS_NewCFunction(ctx, js_print_warn, "warn", 1));

  JS_SetPropertyStr(ctx, global_obj, "console", console);
}

void WeexContextQJS::initGlobalContextFunctions() {
  static const JSCFunctionListEntry js_fib_funcs[] = {
      JS_CFUNC_DEF("callNative", 3, js_CallNative),
      JS_CFUNC_DEF("callNativeModule", 5, js_CallNativeModule),
      JS_CFUNC_DEF("callNativeComponent", 5, js_CallNativeComponent),
      JS_CFUNC_DEF("callAddElement", 5, js_CallAddElement),
#if OS_IOS
      JS_CFUNC_DEF("setTimeout", 2, js_SetTimeout),
#endif
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
      JS_CFUNC_DEF("callGCanvasLinkNative", 3, js_GCanvasLinkNative),
      JS_CFUNC_DEF("setIntervalWeex", 3, js_SetIntervalWeex),
      JS_CFUNC_DEF("clearIntervalWeex", 1, js_ClearIntervalWeex),
      JS_CFUNC_DEF("callT3DLinkNative", 2, js_T3DLinkNative),
      JS_CFUNC_DEF("__updateComponentData", 3, js_UpdateComponentData),
  };
  auto *ctx = static_cast<JSContext *>(js_context());
  JS_SetPropertyFunctionList(ctx, JS_GetGlobalObject(ctx), js_fib_funcs,
                             countof(js_fib_funcs));
  bindConsoleLog(ctx);
}
void WeexContextQJS::initInstanceContextFunctions() {
  static const JSCFunctionListEntry js_fib_funcs[] = {
      JS_CFUNC_DEF("nativeLog", 5, js_NativeLog),
      JS_CFUNC_DEF("atob", 1, js_Atob),
      JS_CFUNC_DEF("btoa", 1, js_Btoa),
      JS_CFUNC_DEF("callGCanvasLinkNative", 3, js_GCanvasLinkNative),
      JS_CFUNC_DEF("setIntervalWeex", 3, js_SetIntervalWeex),
      JS_CFUNC_DEF("clearIntervalWeex", 1, js_ClearIntervalWeex),
      JS_CFUNC_DEF("callT3DLinkNative", 2, js_T3DLinkNative),
      JS_CFUNC_DEF("__updateComponentData", 3, js_UpdateComponentData),
      JS_CFUNC_DEF("setNativeTimeout", 2, js_SetNativeTimeout),
      JS_CFUNC_DEF("clearNativeTimeout", 1, js_ClearNativeTimeout),
      JS_CFUNC_DEF("setNativeInterval", 2, js_SetNativeInterval),
      JS_CFUNC_DEF("clearNativeInterval", 1, js_ClearNativeInterval),
  };
  auto *ctx = static_cast<JSContext *>(js_context());
  JS_SetPropertyFunctionList(ctx, JS_GetGlobalObject(ctx), js_fib_funcs,
                             countof(js_fib_funcs));
  bindConsoleLog(ctx);
}
void WeexContextQJS::initFunctionForAppContext() {

}
void WeexContextQJS::initFromParams(void *jsengine_vm,
                                    std::vector<std::pair<std::string, std::string>> params,
                                    bool forAppContext) {
  if (jsengine_vm == nullptr) {
    return;
  }
  auto *runtime = static_cast<JSRuntime *>(jsengine_vm);
  JSContext *context = createContext(runtime);
  SetJSContext(context);
  JS_SetContextOpaque(context, this);
  initWxEnvironment(params, forAppContext, true);

  if (forAppContext) {
    initFunctionForAppContext();
  } else {
    initGlobalContextFunctions();
  }

}
void WeexContextQJS::addExtraOptions(std::vector<std::pair<std::string, std::string>> params) {
  auto *ctx = static_cast<JSContext *>(js_context());
  const JSValue &jsValue = JS_NewObject(ctx);
  const JSValue &object = JS_GetGlobalObject(ctx);
  size_t size = params.size();
  for (const auto &param: params) {

    const std::string &type = param.first;
    const std::string &value = param.second;

    JSCFunctionListEntry jsEntry = JS_PROP_STRING_DEF(
        type.c_str(), value.c_str(), JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetPropertyStr(ctx, jsValue, type.c_str(),
                      JS_NewString(ctx, value.c_str()));
  }
  JS_SetProperty(ctx, object, JS_NewAtom(ctx, "WXExtraOption"), jsValue);
}
void WeexContextQJS::initWxEnvironment(std::vector<std::pair<std::string, std::string>> params,
                                       bool forAppContext,
                                       bool isSave) {
  auto *ctx = static_cast<JSContext *>(js_context());
  const JSValue &jsValue = JS_NewObject(ctx);
  const JSValue &object = JS_GetGlobalObject(ctx);
  size_t size = params.size();
  for (const auto &param : params) {

    const std::string &type = param.first;
    const std::string &value = param.second;

    JSCFunctionListEntry jsEntry = JS_PROP_STRING_DEF(
        type.c_str(), value.c_str(), JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetPropertyStr(ctx, jsValue, type.c_str(),
                      JS_NewString(ctx, value.c_str()));

    static bool hasSet = false;
    if (!hasSet) {
      if ("debugMode" == type && "true" == value) {
        weex::base::LogImplement::getLog()->setDebugMode(true);
        hasSet = true;
      }
    }

    if (isSave) {
      this->init_framework_args_.emplace_back(type, value);
    }
  }
  JS_SetProperty(ctx, object, JS_NewAtom(ctx, "WXEnvironment"), jsValue);
}
WeexContext *WeexContextQJS::cloneWeexContext(const std::string &page_id,
                                              bool initContext,
                                              bool forAppContext) {
  WeexContext *weexContext = new WeexContextQJS(this->script_bridge());
  auto *ctx = static_cast<JSContext *>(js_context());
  JSContext *newContext = createContext(JS_GetRuntime(ctx));
  weexContext->SetJSContext(newContext);
  weexContext->initWxEnvironment(this->init_framework_args_, forAppContext, false);
  if (forAppContext) {
    weexContext->initFunctionForAppContext();
  } else if (initContext) {
    weexContext->initInstanceContextFunctions();
  } else {
    weexContext->initGlobalContextFunctions();
  }
  weexContext->page_id = page_id;
  JS_SetContextOpaque(newContext, weexContext);
  return weexContext;
}
void WeexContextQJS::updateInitFrameworkParams(const std::string &key, const std::string &value) {
  for (auto param : init_framework_args_) {
    if (key == param.first && value != param.second) {
      param.second = value;
    }
  }
}
void WeexContextQJS::Release() {
  if (js_context()) {
    auto context = static_cast<JSContext *>(js_context());
    JS_FreeContext(context);
//    JS_FreeRuntime(JS_GetRuntime(context));
  }
}
JSContext *WeexContextQJS::createContext(JSRuntime *engine_vm) {
  JSRuntime *rt = engine_vm;
  JSContext *ctx = JS_NewContext(rt);
  JSValue i = JS_GetGlobalObject(ctx);
  JS_SetProperty(ctx, i, JS_NewAtom(ctx, "global"), i);
  JS_SetMaxStackSize(rt, 1024 * 100000);
  return ctx;
}
void WeexContextQJS::RunGC(void *engine_vm) {
  if (engine_vm != nullptr) {
    JS_RunGC(static_cast<JSRuntime *>(engine_vm));
  }
}

static JSValue js_GCAndSweep(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  return JS_TRUE;
}

static std::string toJSON(JSContext *ctx, JSValue value) {
  size_t length;
  const char *ret_string = JS_ToCStringLen(ctx, &length, value);
  return ret_string;
}

static JSValue js_CallNative(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }
  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams task(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams callback(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);

  script_bridge->core_side()->CallNative(
      id.value(), task.value(), callback.value());

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallNativeModule(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams module(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams method(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams arguments(ctx, argv[3], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);
  WeexContextQJS::JSParams options(ctx, argv[4], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);

  std::unique_ptr<ValueWithType> result =
      script_bridge->core_side()->CallNativeModule(
          id.value(), module.value(), method.value(), arguments.value(), arguments.size(),
          options.value(), options.size());
  JSValue ret;
  switch (result->type) {
    case ParamsType::VOID: {
      ret = JS_UNDEFINED;
    }
      break;
    case ParamsType::DOUBLE: {
      ret = JS_NewInt64(ctx, result->value.doubleValue);
    }
      break;
    case ParamsType::STRING: {
      const std::string &string = result->value.string->length == 0 ? "" : weex::base::to_utf8(
          result->value.string->content,
          result->value.string->length);
      ret = JS_NewString(ctx, string.c_str());
    }
      break;
    case ParamsType::BYTEARRAYSTRING: {
      const char *str = result->value.byteArray->content;
      ret = JS_NewString(ctx, str);
    }
      break;
    case ParamsType::JSONSTRING: {
      const std::string &string = result->value.string->length == 0 ? ""
                                                                    : weex::base::to_utf8(result->value.string->content,
                                                                                          result->value.string->length);
      ret = JS_ParseJSON(ctx, string.c_str(), string.length(), "");
      free(result->value.string);
    }
      break;
    case ParamsType::BYTEARRAYJSONSTRING: {
      const WeexByteArray *array = result->value.byteArray;
      const char *string = array->content;
      ret = JS_ParseJSON(ctx, string, array->length, "t");
    }
      break;
    case ParamsType::BYTEARRAY: {
#if OS_ANDROID
      wson_parser w(result->value.byteArray->content,
                    result->value.byteArray->length);
      auto string2String = w.toStringUTF8();
      auto jsvalue2 = JS_ParseJSON(ctx,string2String.c_str(),string2String.length(),"t");
      //LOGE("native BYTEARRAY , call native moudle %s",string2String.c_str());
      ret = jsvalue2;
      if (JS_IsException(jsvalue2) || JS_IsError(ctx,jsvalue2)){
        ret = JS_NewString(ctx, string2String.c_str());
      }
      free(result->value.byteArray);
#elif OS_IOS
      ret = JS_UNDEFINED;
#endif

    }
      break;
    default: {
      ret = JS_UNDEFINED;
    }
      break;
  }
  return ret;
}

static JSValue js_CallNativeComponent(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams ref(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams method(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams arguments(ctx, argv[3], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);
  WeexContextQJS::JSParams options(ctx, argv[4], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);

  script_bridge->core_side()->CallNativeComponent(
      id.value(), ref.value(),
      method.value(), arguments.value(), arguments.size(),
      options.value(), options.size());

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallAddElement(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams parent_ref(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams dom_str(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);
  WeexContextQJS::JSParams index_str(ctx, argv[3], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);

  script_bridge->core_side()->AddElement(
      id.value(), parent_ref.value(),
      dom_str.value(), dom_str.size(), index_str.value());
  return JS_NewInt32(ctx, 0);
}

static JSValue js_SetTimeoutNative(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {

  WeexContextQJS::JSParams callback_id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams time(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);

  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge != nullptr)
    script_bridge->core_side()->SetTimeout(callback_id.value(),
                                           time.value());
  return JS_TRUE;
}

static JSValue js_NativeLog(JSContext *ctx, JSValueConst this_val, int argc,
                            JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }
  std::string result = "quickjsLog";

  for (int i = 0; i < argc; ++i) {
    WeexContextQJS::JSParams ref(ctx, argv[i], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
    result += ref.value();
  }

  script_bridge->core_side()->NativeLog(result.c_str());

  return JS_TRUE;
}

static JSValue js_NotifyTrimMemory(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  return JS_TRUE;
}

static JSValue js_MarkupState(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  return JS_TRUE;
}

static JSValue js_Atob(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv) {
  return JS_TRUE;
}

static JSValue js_Btoa(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv) {
  return JS_TRUE;
}

static JSValue js_CallCreateBody(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams dom_str(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);
  script_bridge->core_side()->CreateBody(
      id.value(), dom_str.value(), dom_str.size());

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallUpdateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {

  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams taskChar(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);
  WeexContextQJS::JSParams callBackChar(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);

  int a = script_bridge->core_side()->UpdateFinish(
      id.value(), taskChar.value(), taskChar.size(),
      callBackChar.value(), callBackChar.size());
  return JS_NewInt32(ctx, a);
}

static JSValue js_CallCreateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }
  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  script_bridge->core_side()->CreateFinish(id.value());
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallRefreshFinish(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams task(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams callback(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);

  int i = script_bridge->core_side()->RefreshFinish(
      id.value(), task.value(),
      callback.value());
  return JS_NewInt32(ctx, i);
}

static JSValue js_CallUpdateAttrs(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {

  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams ref(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams data(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);

  script_bridge->core_side()->UpdateAttrs(id.value(), ref.value(), data.value(), data.size());

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallUpdateStyle(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {

  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }
  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams ref(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams data(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_WSON);
  script_bridge->core_side()->UpdateStyle(id.value(), ref.value(), data.value(), data.size());

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallRemoveElement(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams ref(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  script_bridge->core_side()->RemoveElement(id.value(), ref.value());

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallMoveElement(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams ref(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams parent_ref(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams index(ctx, argv[3], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);

  const char *indexChar = (index.value() == nullptr ? "\0" : index.value());

  script_bridge->core_side()->MoveElement(id.value(),
                                          ref.value(),
                                          parent_ref.value(),
                                          atoi(indexChar));

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallAddEvent(JSContext *ctx, JSValueConst this_val, int argc,
                               JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams ref(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams event(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  script_bridge->core_side()->AddEvent(id.value(), ref.value(), event.value());

  return JS_NewInt32(ctx, 0);;
}

static JSValue js_CallRemoveEvent(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams ref(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams event(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  script_bridge->core_side()->RemoveEvent(id.value(), ref.value(), event.value());

  return JS_NewInt32(ctx, 0);;
}

static JSValue js_GCanvasLinkNative(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  return JS_TRUE;
}

static JSValue js_SetIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {

  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }
  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams callback_id(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams time(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);

  int i = script_bridge->core_side()->SetInterval(id.value(), callback_id.value(), time.value());
  return JS_NewInt32(ctx, i);
}

static JSValue js_ClearIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }
  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams callback_id(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  script_bridge->core_side()->ClearInterval(
      id.value(), callback_id.value());
  return JS_TRUE;
}

static JSValue js_T3DLinkNative(JSContext *ctx, JSValueConst this_val, int argc,
                                JSValueConst *argv) {
  return JS_TRUE;
}

static JSValue js_NativeLogContext(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  return js_NativeLog(ctx, this_val, argc, argv);
}

static JSValue js_DisPatchMessage(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  return JS_TRUE;
}

static JSValue js_DispatchMessageSync(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  return JS_TRUE;
}

static JSValue js_PostMessage(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  return JS_TRUE;
}

static void setNativeTimer(int32_t funcId,
                           int32_t timeoutNumber,
                           JSValueConst this_obj,
                           WeexContextQJS *weexContext,
                           bool repeat) {
  auto *ctx = static_cast<JSContext *>(weexContext->js_context());
  auto context_timer_manager = weexContext->get_timer_manager();
  weex::base::MessageLoop::GetCurrent()->PostDelayedTask(
      [id = funcId,
          jsContext = weexContext,
          timer_manager = context_timer_manager,
          re = repeat,
          timeout = timeoutNumber,
          thisObj = JS_DupValue(ctx, this_obj)] {
        JSValue function = timer_manager->get_timer_function(id);
        if(JS_IsUndefined(function)){
          return;
        }
        auto *context = static_cast<JSContext *>(jsContext->js_context());
        if (!JS_IsFunction(context, function)) {
          return;
        }
        JSValue jsRet = JS_Call(context, function, thisObj, 0, nullptr);
        if (JS_IsException(jsRet)) {
          const JSValue &exception = JS_GetException(context);
          const char *exception_info = JS_ToCString(context, exception);
          JSValue name = JS_GetPropertyStr(context, exception, "name");
          const char *error_name = JS_ToCString(context, name);
          JSValue stack = JS_GetPropertyStr(context, exception, "stack");
          const char *stack_str = JS_ToCString(context, stack);

          std::ostringstream exception_msg;
          exception_msg << "JS Exception : "
                        << (exception_info == nullptr ? "null" : exception_info)
                        << std::endl;
          if (error_name != nullptr) {
            exception_msg << "exception name : " << error_name << std::endl;
          }

          if (stack_str != nullptr) {
            exception_msg << "exception stack : " << stack_str << std::endl;
          }
          JS_FreeValue(context, exception);
          JS_FreeValue(context, name);
          JS_FreeValue(context, stack);
          LOGE("Native Timer Exception %s", exception_msg.str().c_str());
        }

        int ret = 0;
        JSContext *x;
        do {
          ret = JS_ExecutePendingJob(JS_GetRuntime(context), &x);
        } while (ret > 0);

        if (re) {
          setNativeTimer(id, timeout, thisObj, jsContext, re);
        } else {
          timer_manager->remove_timer(id);
          if (JS_IsFunction(context, function)) {
            JS_FreeValue(context, function);
          }
        }
        JS_FreeValue(context, thisObj);
      },
      timeoutNumber);
}

static JSValue addNativeTimer(JSContext *ctx, JSValueConst this_val,
                              int argc, JSValueConst *argv, bool repeat) {
  void *weexContextVoid = JS_GetContextOpaque(ctx);
  if (weexContextVoid == nullptr) {
    return JS_FALSE;
  }
  auto weexContext = static_cast<WeexContextQJS *>(weexContextVoid);
  //0:Func 1:Timeout
  if (argc < 2) {
    return JS_FALSE;
  }

  auto jsFunction = argv[0];
  if (!JS_IsFunction(ctx, jsFunction)) {
    return JS_FALSE;
  }

  auto timeout = argv[1];

  if (!JS_IsNumber(timeout)) {
    return JS_FALSE;
  }

  int32_t timeoutNumber;
  JS_ToInt32(ctx, &timeoutNumber, timeout);
  auto timer_manager = weexContext->get_timer_manager();
  uint32_t funcId = timer_manager->gen_timer_function_id();
  timer_manager->add_timer(funcId, JS_DupValue(ctx, jsFunction));
  setNativeTimer(funcId, timeoutNumber, this_val, weexContext, repeat);
  return JS_NewInt32(ctx, funcId);
}

#ifdef OS_IOS
static JSValue js_SetTimeout(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv)
{
    if (argc >= 2)
    {
        if (JS_IsFunction(ctx, argv[0]) &&
            JS_IsNumber(argv[1]))
        {
            int32_t interval_ms;
            JS_ToInt32(ctx, &interval_ms, argv[1]);
            JSValue func = JS_DupValue(ctx, argv[0]);
            
            WXQJSSetTimeOut(ctx, func, this_val, interval_ms);
        }
    }
    return JS_UNDEFINED;
}
#endif

static JSValue js_SetNativeTimeout(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  return addNativeTimer(ctx, this_val, argc, argv, false);
}

static JSValue js_SetNativeInterval(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  return addNativeTimer(ctx, this_val, argc, argv, true);
}

static JSValue js_ClearNativeTimeout(JSContext *ctx, JSValueConst this_val,
                                     int argc, JSValueConst *argv) {
  void *weexContextVoid = JS_GetContextOpaque(ctx);
  if (weexContextVoid == nullptr) {
    return JS_FALSE;
  }
  auto weexContext = static_cast<WeexContextQJS *>(weexContextVoid);
  auto funcId = argv[1];
  if (!JS_IsNumber(funcId)) {
    return JS_FALSE;
  }
  int32_t id;
  JS_ToInt32(ctx, &id, funcId);
  auto timer_manager = weexContext->get_timer_manager();
  weex::base::MessageLoop::GetCurrent()->PostTask([m_timer_manager = timer_manager , m_ctx = ctx, m_id = id]{
      JSValue timerFunction = m_timer_manager->remove_timer(m_id);
      if (JS_IsFunction(m_ctx, timerFunction)) {
        JS_FreeValue(m_ctx, timerFunction);
      }
  });
  return JS_TRUE;
}

static JSValue js_ClearNativeInterval(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  return js_ClearNativeTimeout(ctx, this_val, argc, argv);
}

// For data render
static JSValue js_UpdateComponentData(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  std::shared_ptr<WeexCore::ScriptBridge> script_bridge = bridge(ctx);
  if (script_bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexContextQJS::JSParams id(ctx, argv[0], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams cid(ctx, argv[1], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);
  WeexContextQJS::JSParams json_data(ctx, argv[2], WeexContextQJS::JSParams::PARAMS_TYPE_JSON);

  script_bridge->core_side()->UpdateComponentData(id.value(),
                                                  cid.value(),
                                                  json_data.value());

  return JS_TRUE;
}
void WeexContextQJS::JSParams::set_value(JSContext *context, JSValue value) {

#if OS_ANDROID
  if (param_type == PARAMS_TYPE_WSON) {
    wson_buffer_ = toWsonBuffer(context, value);
    value_ = (char *) (wson_buffer_->data);
    length_ = wson_buffer_->position;
  } else {
    ctx_ = context;
    if (!JS_IsString(value)) {
      value = JS_JSONStringify(context, value, JS_UNDEFINED,
                               JS_NewInt32(context, 0));
    }
    value_ = const_cast<char *>(JS_ToCStringLen(context,
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
WeexContextQJS::JSParams::~JSParams() {
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
WeexContextQJS::JSParams::JSParams(JSContext *context, JSValue value, int type) {
  this->param_type = type;
  ctx_ = nullptr;
  value_ = nullptr;
  length_ = 0;
#if OS_ANDROID
  wson_buffer_ = nullptr;
#endif
  set_value(context, value);
}
