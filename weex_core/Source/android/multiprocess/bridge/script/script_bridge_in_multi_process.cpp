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
// Created by yxp on 2018/6/15.
//

#include <android/base/string/string_utils.h>
#include <unistd.h>
#include "base/thread/waitable_event.h"
#include "base/make_copyable.h"
#include "script_bridge_in_multi_process.h"

#include "core_side_in_multi_process.h"
#include "android/multiprocess/weex_progress_env.h"
#include "android/multiprocess/weex_ipc_server.h"
#include "core/bridge/script/script_side_in_simple.h"
#include "core/runtime/weex_runtime.h"
#include "third_party/IPC/IPCArguments.h"
#include "third_party/IPC/IPCException.h"
#include "third_party/IPC/IPCHandler.h"
#include "third_party/IPC/IPCMessageJS.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/IPCString.h"

static WeexJSServer *server = nullptr;

static unsigned long parseUL(const char *s) {
  unsigned long val;
  errno = 0;
  val = strtoul(s, nullptr, 10);
  if (errno) {
    LOGE("failed to parse ul: %s %s", s, strerror(errno));
    _exit(1);
  }
  return val;
}

struct ThreadData {
  int fd;
  int fd_client;
  bool enableTrace;
  char *crashFileName;
};

static void *threadEntry(void *_td) {
  auto *td = static_cast<ThreadData *>(_td);
  //  server = new weex::IPCServer(static_cast<int>(td->fd),
  //  static_cast<bool>(td->enableTrace));
  server = new WeexJSServer(static_cast<int>(td->fd), static_cast<int>(td->fd_client),
                            static_cast<bool>(td->enableTrace), td->crashFileName);
  // Register handler for bridge
  nice(6);
  try {
    server->loop();
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
    _exit(1);
  }
  return static_cast<void **>(nullptr);
}

__attribute__((visibility("default"))) extern "C" int serverMain(int argc, char **argv) {
  unsigned long fd;
  unsigned long fd_client = 0;
  unsigned long enableTrace;
  if (argc < 4) {
    LOGE("argc is not correct");
    _exit(1);
  }
  fd = parseUL(argv[1]);
  fd_client = parseUL(argv[2]);
  enableTrace = parseUL(argv[3]);
  char *fileName = argv[4];
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  pthread_attr_setstacksize(&threadAttr, 10 * 1024 * 1024);
  pthread_t thread;
  ThreadData td =
      {static_cast<int>(fd), static_cast<int>(fd_client), static_cast<bool>(enableTrace), fileName};
  pthread_create(&thread, &threadAttr, threadEntry, &td);
  void *rdata;
  pthread_join(thread, &rdata);
  return 0;
}

namespace weex {
namespace bridge {
namespace js {
static inline const char *GetUTF8StringFromIPCArg(IPCArguments *arguments, size_t index) {
  return arguments->getByteArray(index)->length == 0 ? nullptr
                                                     : arguments->getByteArray(index)->content;
}

ScriptBridgeInMultiProcess *ScriptBridgeInMultiProcess::g_instance = NULL;

ScriptBridgeInMultiProcess::ScriptBridgeInMultiProcess() {
  task_queue_thread_ = std::make_unique<weex::base::Thread>(weex::base::MessageLoop::DEFAULT);
  task_queue_thread_->Start();
  LOGE("thread isStarted and task_queue_thread_ %u", pthread_self());
  set_script_side(new WeexCore::bridge::script::ScriptSideInSimple(false));
  set_core_side(new CoreSideInMultiProcess());
}

ScriptBridgeInMultiProcess::~ScriptBridgeInMultiProcess() {}

static void FillVectorOfValueWithType(std::vector<VALUE_WITH_TYPE *> &params,
                                      IPCArguments *arguments, size_t start,
                                      size_t end) {
  for (size_t i = start; i < end; ++i) {
    auto value = new ValueWithType;
    switch (arguments->getType(i)) {
      case IPCType::DOUBLE: {
        value->type = ParamsType::DOUBLE;
        value->value.doubleValue = arguments->get<double>(i);
      }
        break;
      case IPCType::STRING: {
        const IPCString *ipcstr = arguments->getString(i);
        size_t size = ipcstr->length * sizeof(uint16_t);
        auto temp = (WeexString *) malloc(size + sizeof(WeexString));
        memset(temp, 0, size + sizeof(WeexString));
        temp->length = ipcstr->length;
        memcpy(temp->content, ipcstr->content, size);
        value->type = ParamsType::STRING;
        value->value.string = temp;
      }
        break;
      case IPCType::JSONSTRING: {
        const IPCString *ipcstr = arguments->getString(i);
        size_t size = ipcstr->length * sizeof(uint16_t);
        auto temp = (WeexString *) malloc(size + sizeof(WeexString));
        memset(temp, 0, size + sizeof(WeexString));
        temp->length = ipcstr->length;
        memcpy(temp->content, ipcstr->content, size);
        value->type = ParamsType::JSONSTRING;
        value->value.string = temp;
      }
        break;
      case IPCType::BYTEARRAY: {
        const IPCByteArray *array = arguments->getByteArray(i);
        size_t size = array->length * sizeof(char);
        auto temp = (WeexByteArray *) malloc(size + sizeof(WeexByteArray));
        memset(temp, 0, size + sizeof(WeexByteArray));
        temp->length = array->length;
        memcpy(temp->content, array->content, size);
        value->type = ParamsType::BYTEARRAY;
        value->value.byteArray = temp;
      }
        break;
      default: {
        value->type = ParamsType::JSUNDEFINED;
      }
        break;
    }
    params.push_back(value);
  }
}

static void ClearVectorOfValueWithType(const std::vector<VALUE_WITH_TYPE *> &params) {
  for (auto &param : params) {
    switch (param->type) {
      case ParamsType::STRING:
      case ParamsType::JSONSTRING:free(param->value.string);
        break;
      case ParamsType::BYTEARRAY:free(param->value.byteArray);
        break;
      default:break;
    }
    delete param;
  }
}

void ScriptBridgeInMultiProcess::RegisterIPCCallback(IPCHandler *handler) {
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::INITFRAMEWORK),
                           InitFramework);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::INITAPPFRAMEWORK),
                           InitAppFramework);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::CREATEAPPCONTEXT),
                           CreateAppContext);
  handler->registerHandler(
      static_cast<uint32_t>(IPCJSMsg::EXECJSONAPPWITHRESULT),
      ExecJSOnAppWithResult);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::CALLJSONAPPCONTEXT),
                           CallJSOnAppContext);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::DESTORYAPPCONTEXT),
                           DestroyAppContext);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSSERVICE),
                           ExecJSService);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::TAKEHEAPSNAPSHOT),
                           TakeHeapSnapshot);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECTIMERCALLBACK),
                           ExecTimerCallback);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJS), ExecJS);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSWITHRESULT),
                           ExecJSWithResult);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSWITHCALLBACK),
                           ExecJSWithCallback);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::CREATEINSTANCE),
                           CreateInstance);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::DESTORYINSTANCE),
                           DestroyInstance);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSONINSTANCE),
                           ExecJSOnInstance);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::UPDATEGLOBALCONFIG),
                           UpdateGlobalConfig);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::UpdateInitFrameworkParams),
                           UpdateInitFrameworkParams);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::SETLOGLEVEL),
                           setLogType);
  handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::COMPILEQUICKJSBIN),
                           compileQuickJSBin);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::InitFramework(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::InitFramework");
  // Source
  auto source = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));

  // Params
  size_t startCount = 1;
  size_t count = arguments->getCount();
  std::vector<std::pair<std::string, std::string>> params;
  for (size_t i = startCount; i < count; i += 2) {
    if (arguments->getType(i) != IPCType::BYTEARRAY) {
      continue;
    }
    if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
      continue;
    }
    const IPCByteArray *ba = arguments->getByteArray(1 + i);
    const IPCByteArray *ba_type = arguments->getByteArray(i);

    auto key = std::string(ba_type->content);
    auto value = std::string(ba->content);

    if (key == "debugMode") {
      weex::base::LogImplement::getLog()->setDebugMode(value == "true");
    } else if (key == "engine_type") {
      char *end;
      long type = strtol(value.c_str(),
                         &end,
                         10);
      LOGE("weex env set engine_type is %s %d", value.c_str(), type);
      WeexRuntimeManager::Instance()->set_engine_type(type);
    } else if (key == "enableBackupThread") {
      WeexRuntimeManager::Instance()->set_enable_backup_thread(value == "true");
    } else if (key == "enableBackupThreadCache") {
      WeexRuntimeManager::Instance()->set_enable_backup_thread_cache(value == "true");
    } else if (key == "enableAlarmSignal") {
      WeexProgressEnv::getEnv()->enableHandleAlarmSignal(value == "true");
    }
    params.emplace_back(key, value);
  }
  base::MessageLoop *kPLoop = ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop();
  kPLoop
      ->PostTask(weex::base::MakeCopyable(
          [s = std::move(source), p = std::move(params)] {
            Instance()->script_side()->InitFramework(s.get(), p);
          }));
  return createInt32Result(1);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::InitAppFramework(
    IPCArguments *arguments) {
  auto id = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto js = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));

  if (id == nullptr) {
    return createInt32Result(static_cast<int32_t>(false));
  }
  // Params
  size_t startCount = 2;
  size_t count = arguments->getCount();
  std::vector<std::pair<std::string, std::string>> params;
  for (size_t i = startCount; i < count; i += 2) {
    if (arguments->getType(i) != IPCType::BYTEARRAY) {
      continue;
    }
    if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
      continue;
    }
    const IPCByteArray *ba = arguments->getByteArray(1 + i);
    const IPCByteArray *ba_type = arguments->getByteArray(i);
    params.emplace_back(std::make_pair(ba_type->content, ba->content));
  }

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [id_s = std::move(id), js_s = std::move(js), p = std::move(params)] {
            Instance()->script_side()->InitAppFramework(id_s.get(), js_s.get(), p);
          }));

  return createInt32Result(1);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::CreateAppContext(
    IPCArguments *arguments) {
  const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
  const char *js = GetUTF8StringFromIPCArg(arguments, 1);

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));

  return createInt32Result(
      Instance()->script_side()->CreateAppContext(instanceID, js));
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSOnAppWithResult(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::ExecJSONAppWithResult");
  const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
  const char *js = GetUTF8StringFromIPCArg(arguments, 1);
  const std::unique_ptr<WeexJSResult>
      &ptr = Instance()->script_side()->ExecJSOnAppWithResult(instanceID, js);

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));

  return createByteArrayResult(ptr->data.get(), ptr->length);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::CallJSOnAppContext(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::CallJSONAppContext");
  const char *instanceId = GetUTF8StringFromIPCArg(arguments, 0);
  const char *func = GetUTF8StringFromIPCArg(arguments, 1);

  std::vector<VALUE_WITH_TYPE *> params;
  FillVectorOfValueWithType(params, arguments, 2, arguments->getCount());
  auto result =
      Instance()->script_side()->CallJSOnAppContext(instanceId, func, params);
  ClearVectorOfValueWithType(params);

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));

  return createInt32Result(result);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::DestroyAppContext(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::DestroyAppContext");
  const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
  if (strlen(instanceID) == 0) {
    return createInt32Result(static_cast<int32_t>(false));
  }
  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));
  return createInt32Result(
      Instance()->script_side()->DestroyAppContext(instanceID));
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSService(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::ExecJSService");
  auto source = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [s = std::move(source)] {
            Instance()->script_side()->ExecJsService(s.get());
          }));

  return createInt32Result(1);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecTimerCallback(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::ExecTimerCallback");
  auto source = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [s = std::move(source)] {
            Instance()->script_side()->ExecTimeCallback(s.get());
          }));

  return createVoidResult();
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJS(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::ExecJS");

  auto instanceId = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto namespaceStr = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto func = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));

  std::vector<VALUE_WITH_TYPE *> params;
  FillVectorOfValueWithType(params, arguments, 3, arguments->getCount());

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [id = std::move(instanceId), name = std::move(namespaceStr), f = std::move(func),
              p = std::move(params)] {
            Instance()->script_side()->ExecJS(id.get(), name.get(), f.get(), p);
            ClearVectorOfValueWithType(p);
          }));

  return createInt32Result(1);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSWithResult(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::ExecJSWithResult");
  const char *instanceId = GetUTF8StringFromIPCArg(arguments, 0);
  const char *namespaceStr = GetUTF8StringFromIPCArg(arguments, 1);
  const char *func = GetUTF8StringFromIPCArg(arguments, 2);

  std::vector<VALUE_WITH_TYPE *> params;
  FillVectorOfValueWithType(params, arguments, 3, arguments->getCount());
  const std::unique_ptr<WeexJSResult> &ptr = Instance()->script_side()->ExecJSWithResult(
      instanceId, namespaceStr, func, params);
  ClearVectorOfValueWithType(params);
  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));
  return createByteArrayResult(ptr->data.get(), ptr->length);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSWithCallback(
    IPCArguments *arguments) {
  const char *instanceId = GetUTF8StringFromIPCArg(arguments, 0);
  const char *namespaceStr = GetUTF8StringFromIPCArg(arguments, 1);
  const char *func = GetUTF8StringFromIPCArg(arguments, 2);
  // pass callback_id before params
  long id = arguments->get<int64_t>(3);

  std::vector<VALUE_WITH_TYPE *> params;
  FillVectorOfValueWithType(params, arguments, 4, arguments->getCount());
  Instance()->script_side()->ExecJSWithCallback(
      instanceId, namespaceStr, func, params, id);
  ClearVectorOfValueWithType(params);
  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));
  return createVoidResult();
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::CreateInstance(
    IPCArguments *arguments) {

  auto instanceID = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto func = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto script = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  const int script_size = arguments->getByteArray(2)->length;
  auto opts = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
  auto initData = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 4));
  auto extendsApi = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 5));

  size_t startCount = 6;
  size_t count = arguments->getCount();
  std::vector<std::pair<std::string, std::string>> params;
  for (size_t i = startCount; i < count; i += 2) {
    if (arguments->getType(i) != IPCType::BYTEARRAY) {
      continue;
    }
    if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
      continue;
    }
    const IPCByteArray *ba = arguments->getByteArray(1 + i);
    const IPCByteArray *ba_type = arguments->getByteArray(i);
    params.emplace_back(std::make_pair(ba_type->content, ba->content));
  }
  LOG_TLOG("jsEngine",
           "ScriptBridgeInMultiProcess::CreateInstance and Id is : %s",
           instanceID.get());

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [id = std::move(instanceID), fun = std::move(func), s = std::move(script),
              sl = script_size, op = std::move(opts), init = std::move(initData),
              exe = std::move(extendsApi), p = std::move(params)] {
            Instance()->script_side()->CreateInstance(
                id.get(), fun.get(), s.get(), sl, op.get(), init.get(), exe.get(), p);
          }));
  return createInt32Result(1);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::DestroyInstance(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::DestroyInstance");

  auto instanceID = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));

  if (instanceID == nullptr || strlen(instanceID.get()) == 0) {
    LOGE("DestoryInstance instanceId is NULL");
    return createInt32Result(static_cast<int32_t>(false));
  }
  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [id = std::move(instanceID)] {
            Instance()->script_side()->DestroyInstance(id.get());
          }));
  return createInt32Result(1);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSOnInstance(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::ExecJSONInstance");
  const int script_size = arguments->getByteArray(1)->length;
  int type = arguments->get<int32_t>(2);
  auto instanceID = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto script = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [id = std::move(instanceID), s = std::move(script), size = script_size,
              t = type] {
            Instance()->script_side()->ExecJSOnInstance(id.get(), s.get(), size, t);
          }));
  return createByteArrayResult("", 0);
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::UpdateGlobalConfig(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::UpdateGlobalConfig");
  const char *configString = GetUTF8StringFromIPCArg(arguments, 0);
  Instance()->script_side()->UpdateGlobalConfig(configString);

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));

  return createVoidResult();
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::UpdateInitFrameworkParams(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::UpdateInitFrameworkParams");
  const char *key = GetUTF8StringFromIPCArg(arguments, 0);
  const char *value = GetUTF8StringFromIPCArg(arguments, 1);
  const char *desc = GetUTF8StringFromIPCArg(arguments, 2);
  Instance()->script_side()->UpdateInitFrameworkParams(key, value, desc);
  LOGD("ScriptBridgeInMultiProcess::UpdateInitFrameworkParams End");

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));
  return createVoidResult();
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::setLogType(
    IPCArguments *arguments) {
  LOGD("ScriptBridgeInMultiProcess::setLogType");
  int type = arguments->get<int32_t>(0);
  int perf = arguments->get<int32_t>(1);
  Instance()->script_side()->SetLogType(type, perf == 1);
  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));
  return createVoidResult();
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::compileQuickJSBin(
    IPCArguments *arguments) {
  const char *key = GetUTF8StringFromIPCArg(arguments, 0);
  const char *script = GetUTF8StringFromIPCArg(arguments, 1);
  Instance()->script_side()->CompileQuickJSBin(key, script);

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));

  return createVoidResult();
}

std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::TakeHeapSnapshot(
    IPCArguments *arguments) {

  ScriptBridgeInMultiProcess::Instance()
      ->js_thread()
      ->message_loop()
      ->PostTask(weex::base::MakeCopyable(
          [] {}));
  return createVoidResult();
}

}  // namespace js
}  // namespace bridge
}  // namespace weex
