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

#ifndef WEEX_PROJECT_SCRIPT_SIDE_IN_QJS_H
#define WEEX_PROJECT_SCRIPT_SIDE_IN_QJS_H
#include "core/bridge/script_bridge.h"
#include "include/qjs/quickjs.h"
#include <wson.h>
#include <unordered_map>
#include "wson_parser.h"

namespace WeexCore {
namespace bridge {
namespace script {
class ScriptSideInQJS : public ScriptBridge::ScriptSide {
 public:
  class JSParams {
   public:
    explicit JSParams(JSContext* context, JSValue value, int type);
    void set_value(JSContext* context, JSValue value);

    ~JSParams();
    const char* value() {
      return value_;
    }

    const size_t size() {
      return length_;
    }

   public:
    static const int PARAMS_TYPE_JSON = 0;
    static const int PARAMS_TYPE_WSON = 1;

   private:
    int param_type;
    char* value_;
    size_t length_;
    JSContext* ctx_;
#if OS_ANDROID
    wson_buffer* wson_buffer_;
#endif
  };
  ScriptSideInQJS();
  int InitFramework(const char* script,
                    std::vector<std::pair<std::string, std::string>> params) override;

  int InitAppFramework(const char *instanceId, const char *appFramework,
                       std::vector<std::pair<std::string, std::string>> params) final { return 0; }

  int CreateAppContext(const char* instanceId, const char* jsBundle) final { return 0; };

  std::unique_ptr<WeexJSResult> ExecJSOnAppWithResult(const char* instanceId,
                                                      const char* jsBundle) override;

  int CallJSOnAppContext(const char* instanceId, const char* func,
                         std::vector<VALUE_WITH_TYPE*> &params) final { return 0; };

  int DestroyAppContext(const char* instanceId) final { return 0; };

  int ExecJsService(const char* source) override;

  int ExecTimeCallback(const char* source) final { return 0; };

  int ExecJS(const char *instanceId, const char *nameSpace,
             const char *func,
             const std::vector<VALUE_WITH_TYPE *> &params) override;

  std::unique_ptr<WeexJSResult> ExecJSWithResult(const char* instanceId, const char* nameSpace,
                                                 const char* func,
                                                 std::vector<VALUE_WITH_TYPE*> &params) override;

  void ExecJSWithCallback(const char* instanceId, const char* nameSpace,
                          const char* func,
                          std::vector<VALUE_WITH_TYPE*> &params,
                          long callback_id) override;

  int CreateInstance(const char *instanceId,
                     const char *func,
                     const char *script,
                     const int script_size,
                     const char *opts,
                     const char *initData,
                     const char *extendsApi,
                     std::vector<std::pair<std::string, std::string>> params) override;

  std::unique_ptr<WeexJSResult> ExecJSOnInstance(const char *instanceId,
                                                 const char *script,
                                                 const int script_size,
                                                 int type) override;

  int DestroyInstance(const char* instanceId) override;

  int UpdateGlobalConfig(const char* config) final { return 1; };

  int UpdateInitFrameworkParams(const std::string &key,
                                const std::string &value,
                                const std::string &desc) override;

  void SetLogType(const int logLevel, const bool isPerf) override;

  void CompileQuickJSBin(const char *key, const char *script) override {}
//  int64_t JsAction(long ctxContainer, int32_t jsActionType, const char* arg) final { return 0; }

 private:
  struct QJSByteCode {
    uint8_t* buf;
    size_t buf_len;
  };
  JSContext* CreateContext(JSRuntime* engine_vm);
  void InitWXEnvironment(std::vector<std::pair<std::string, std::string>>& params,
                         JSContext* context,
                         bool is_save);
  void InitWXEnvironment(JSContext* context);
  void InitGlobalContextFunctions();
  void InitInstanceContextFunctions(JSContext* context);
  std::unique_ptr<WeexJSResult> exeJSWithResult(const char* instanceId,
                                                const char* nameSpace,
                                                const char* func,
                                                std::vector<VALUE_WITH_TYPE*> &params);
  static void FinishQJSPendingJob(JSRuntime* rt);
  static void ConvertJSValueToWeexJSResult(JSContext* ctx,
                                           JSValue &ret,
                                           WeexJSResult* jsResult);
  int ExecuteScript(JSContext* ctx,
                    const char* instance_id,
                    const char* script,
                    bool& use_qjs_byte_code);
  int ExecuteExtendsScript(JSContext* ctx,
                           const char* instance_id,
                           const char* script,
                           bool& use_qjs_byte_code);

  std::vector<std::pair<std::string, std::string>> init_framework_args_;
  std::map<const char*, JSContext*> js_context_map_;
  JSRuntime* js_engine_vm_ = nullptr;
  JSContext* global_context_ = nullptr;
  std::unordered_map<std::string, QJSByteCode> cached_qjs_byte_code_;
  uint8_t* cached_extends_qjs_byte_code_ = nullptr;
};
}  // namespace script
}  // namespace bridge
}  // namespace WeexCore
#endif //WEEX_PROJECT_SCRIPT_SIDE_IN_QJS_H
