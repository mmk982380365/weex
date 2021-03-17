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

#ifndef WEEX_PROJECT_SCRIPT_SIDE_QJS_BINDING_H
#define WEEX_PROJECT_SCRIPT_SIDE_QJS_BINDING_H

#include "android/bridge/script/qjs/script_side_in_qjs.h"
#include "base/string_util.h"
#include "base/utils/log_utils.h"
#include "core/bridge/script_bridge.h"
#include "core/manager/weex_core_manager.h"
#include "include/qjs/quickjs.h"
#include "wson.h"
#include "wson_parser.h"

namespace WeexCore {
namespace bridge {
namespace script {

#define UNICORN_WEEX_RENDER_ACTION \
  (reinterpret_cast<void (*)(std::string, std::string, JSContext*, JSValueConst*, int)> \
  (WeexCore::WeexCoreManager::Instance()->unicorn_weex_action_ptr()))

static WeexCore::ScriptBridge* script_bridge(JSContext* ctx) {
  if (ctx == nullptr) {
    return nullptr;
  }

  void* Opaque = JS_GetContextOpaque(ctx);
  if (Opaque == nullptr) {
    return nullptr;
  }

  auto* kPContext = static_cast<WeexCore::bridge::script::ScriptSideInQJS*>(Opaque);
  if (kPContext == nullptr) {
    return nullptr;
  }

  return kPContext->bridge();
}

static void ReportException(JSContext* context,
                            const std::string &funcName,
                            const std::string &page_id, WeexCore::ScriptBridge* bridge) {
  const JSValue &exception = JS_GetException(context);
  const char* exception_info = JS_ToCString(context, exception);
  JSValue name = JS_GetPropertyStr(context, exception, "name");
  const char* error_name = JS_ToCString(context, name);
  JSValue stack = JS_GetPropertyStr(context, exception, "stack");
  const char* stack_str = JS_ToCString(context, stack);

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

static JSValue js_CallNativeComponent(JSContext* ctx, JSValueConst this_val,
                                      int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "callNativeComponent", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallAddElement(JSContext* ctx, JSValueConst this_val,
                                 int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "addElement", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallCreateBody(JSContext* ctx, JSValueConst this_val,
                                 int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "createBody", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallCreateFinish(JSContext* ctx, JSValueConst this_val,
                                   int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }
  UNICORN_WEEX_RENDER_ACTION("dom", "createFinish", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallUpdateAttrs(JSContext* ctx, JSValueConst this_val,
                                  int argc, JSValueConst* argv) {

  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "updateAttributes", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallUpdateStyle(JSContext* ctx, JSValueConst this_val,
                                  int argc, JSValueConst* argv) {

  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "updateStyle", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallRemoveElement(JSContext* ctx, JSValueConst this_val,
                                    int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "removeElement", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallMoveElement(JSContext* ctx, JSValueConst this_val,
                                  int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "moveElement", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallAddEvent(JSContext* ctx, JSValueConst this_val, int argc,
                               JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "addEvent", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);;
}

static JSValue js_CallRemoveEvent(JSContext* ctx, JSValueConst this_val,
                                  int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  UNICORN_WEEX_RENDER_ACTION("dom", "removeEvent", ctx, argv, argc);
  return JS_NewInt32(ctx, 0);;
}

static JSValue js_CallNative(JSContext* ctx, JSValueConst this_val, int argc,
                             JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      id(ctx, argv[0], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      task(ctx, argv[1], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      callback(ctx, argv[2], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);

  bridge->core_side()->CallNative(
      id.value(), task.value(), callback.value());

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallNativeModule(JSContext* ctx, JSValueConst this_val,
                                   int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      id(ctx, argv[0], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      module(ctx, argv[1], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      method(ctx, argv[2], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams arguments
      (ctx, argv[3], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_WSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      options(ctx, argv[4], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_WSON);

  auto result =
      bridge->core_side()->CallNativeModule(
          id.value(), module.value(), method.value(), arguments.value(), arguments.size(),
          options.value(), options.size());
  JSValue ret;
  switch (result->type) {
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
    case ParamsType::JSONSTRING: {
      const std::string &string = result->value.string->length == 0 ? ""
                                                                    : weex::base::to_utf8(result->value.string->content,
                                                                                          result->value.string->length);
      ret = JS_ParseJSON(ctx, string.c_str(), string.length(), "");
      free(result->value.string);
    }
      break;
    case ParamsType::BYTEARRAYJSONSTRING: {
      const WeexByteArray* array = result->value.byteArray;
      const char* string = array->content;
      ret = JS_ParseJSON(ctx, string, array->length, "t");
    }
      break;
    case ParamsType::BYTEARRAY: {
#if OS_ANDROID
      wson_parser w(result->value.byteArray->content,
                    result->value.byteArray->length);
      auto string2String = w.toStringUTF8();
      auto jsvalue =
          JS_NewString(ctx, string2String.c_str());
      free(result->value.byteArray);
      ret = jsvalue;
#elif OS_IOS
      ret = JS_UNDEFINED;
#endif

    }
      break;
    default: { ret = JS_UNDEFINED; }
      break;
  }
  return ret;
}

static JSValue js_SetTimeoutNative(JSContext* ctx, JSValueConst this_val,
                                   int argc, JSValueConst* argv) {

  WeexCore::bridge::script::ScriptSideInQJS::JSParams callback_id
      (ctx, argv[0], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      time(ctx, argv[1], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);

  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge != nullptr)
    bridge->core_side()->SetTimeout(callback_id.value(),
                                    time.value());
  return JS_TRUE;
}

static JSValue js_CallUpdateFinish(JSContext* ctx, JSValueConst this_val,
                                   int argc, JSValueConst* argv) {

  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexCore::bridge::script::ScriptSideInQJS::JSParams id(ctx, argv[0],
                                                         WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      taskChar(ctx, argv[1], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_WSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams callBackChar
      (ctx, argv[2], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_WSON);

  int a = bridge->core_side()->UpdateFinish(
      id.value(), taskChar.value(), taskChar.size(),
      callBackChar.value(), callBackChar.size());
  return JS_NewInt32(ctx, a);
}

static JSValue js_CallRefreshFinish(JSContext* ctx, JSValueConst this_val,
                                    int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }

  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      id(ctx, argv[0], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      task(ctx, argv[1], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      callback(ctx, argv[2], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);

  int i = bridge->core_side()->RefreshFinish(
      id.value(), task.value(),
      callback.value());
  return JS_NewInt32(ctx, i);
}

static JSValue js_SetIntervalWeex(JSContext* ctx, JSValueConst this_val,
                                  int argc, JSValueConst* argv) {

  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      id(ctx, argv[0], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams callback_id
      (ctx, argv[1], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      time(ctx, argv[2], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);

  int i = bridge->core_side()->SetInterval(id.value(), callback_id.value(), time.value());
  return JS_NewInt32(ctx, i);
}

static JSValue js_ClearIntervalWeex(JSContext* ctx, JSValueConst this_val,
                                    int argc, JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }
  WeexCore::bridge::script::ScriptSideInQJS::JSParams
      id(ctx, argv[0], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  WeexCore::bridge::script::ScriptSideInQJS::JSParams callback_id
      (ctx, argv[1], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
  bridge->core_side()->ClearInterval(
      id.value(), callback_id.value());
  return JS_TRUE;
}

// --- log implementation start ---
// Log = 1,
// Warning = 2,
// Error = 3,
// Debug = 4,
// Info = 5,
static JSValue js_print(JSContext* ctx, JSValueConst this_val, int argc,
                        JSValueConst* argv, int level) {
  if (argc == 0) {
    return JS_TRUE;
  }

  JSValueConst msg = argv[0];
  const char* str = nullptr;

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

static JSValue js_print_log(JSContext* ctx, JSValueConst this_val, int argc,
                            JSValueConst* argv) {
  return js_print(ctx, this_val, argc, argv, 1);
}

static JSValue js_print_info(JSContext* ctx, JSValueConst this_val, int argc,
                             JSValueConst* argv) {
  return js_print(ctx, this_val, argc, argv, 5);
}

static JSValue js_print_debug(JSContext* ctx, JSValueConst this_val, int argc,
                              JSValueConst* argv) {
  return js_print(ctx, this_val, argc, argv, 4);
}

static JSValue js_print_error(JSContext* ctx, JSValueConst this_val, int argc,
                              JSValueConst* argv) {
  return js_print(ctx, this_val, argc, argv, 3);
}
static JSValue js_print_warn(JSContext* ctx, JSValueConst this_val, int argc,
                             JSValueConst* argv) {
  return js_print(ctx, this_val, argc, argv, 2);
}

static void BindConsoleLog(JSContext* ctx) {
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

static JSValue js_NativeLog(JSContext* ctx, JSValueConst this_val, int argc,
                            JSValueConst* argv) {
  ScriptBridge* bridge = script_bridge(ctx);
  if (bridge == nullptr) {
    return JS_UNDEFINED;
  }
  std::string result = "quickjsLog";

  for (int i = 0; i < argc; ++i) {
    WeexCore::bridge::script::ScriptSideInQJS::JSParams
        ref(ctx, argv[i], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
    result += ref.value();
  }

  bridge->core_side()->NativeLog(result.c_str());

  return JS_TRUE;
}
// --- log implementation end ---

//static JSValue js_NativeLogContext(JSContext* ctx, JSValueConst this_val,
//                                   int argc, JSValueConst* argv) {
//  return js_NativeLog(ctx, this_val, argc, argv);
//}

// For data render
//static JSValue js_UpdateComponentData(JSContext* ctx, JSValueConst this_val,
//                                      int argc, JSValueConst* argv) {
//  ScriptBridge* bridge = script_bridge(ctx);
//  if (bridge == nullptr) {
//    return JS_UNDEFINED;
//  }
//
//  WeexCore::bridge::script::ScriptSideInQJS::JSParams
//      id(ctx, argv[0], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
//  WeexCore::bridge::script::ScriptSideInQJS::JSParams
//      cid(ctx, argv[1], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
//  WeexCore::bridge::script::ScriptSideInQJS::JSParams json_data
//      (ctx, argv[2], WeexCore::bridge::script::ScriptSideInQJS::JSParams::PARAMS_TYPE_JSON);
//
//  bridge->core_side()->UpdateComponentData(id.value(),
//                                           cid.value(),
//                                           json_data.value());
//
//  return JS_TRUE;
//}

static JSValue js_NotifyTrimMemory(JSContext* ctx, JSValueConst this_val,
                                   int argc, JSValueConst* argv) {
  return JS_TRUE;
}

static JSValue js_MarkupState(JSContext* ctx, JSValueConst this_val, int argc,
                              JSValueConst* argv) {
  return JS_TRUE;
}

static JSValue js_Atob(JSContext* ctx, JSValueConst this_val, int argc,
                       JSValueConst* argv) {
  return JS_TRUE;
}

static JSValue js_Btoa(JSContext* ctx, JSValueConst this_val, int argc,
                       JSValueConst* argv) {
  return JS_TRUE;
}

//static JSValue js_GCanvasLinkNative(JSContext* ctx, JSValueConst this_val,
//                                    int argc, JSValueConst* argv) {
//  return JS_TRUE;
//}
//
//static JSValue js_T3DLinkNative(JSContext* ctx, JSValueConst this_val, int argc,
//                                JSValueConst* argv) {
//  return JS_TRUE;
//}

//static JSValue js_DisPatchMessage(JSContext* ctx, JSValueConst this_val,
//                                  int argc, JSValueConst* argv) {
//  return JS_TRUE;
//}
//
//static JSValue js_DispatchMessageSync(JSContext* ctx, JSValueConst this_val,
//                                      int argc, JSValueConst* argv) {
//  return JS_TRUE;
//}
//
//static JSValue js_PostMessage(JSContext* ctx, JSValueConst this_val, int argc,
//                              JSValueConst* argv) {
//  return JS_TRUE;
//}

}  // namespace script
}  // namespace bridge
}  // namespace WeexCore

#endif //WEEX_PROJECT_SCRIPT_SIDE_QJS_BINDING_H
