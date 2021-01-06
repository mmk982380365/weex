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
// Created by Darin on 12/02/2018.
//

#ifndef WEEXV8_WEEXAPIHEADER_H
#define WEEXV8_WEEXAPIHEADER_H

#include <vector>
#include <set>
#include <map>
#include "WeexApiValue.h"

#ifdef OS_ANDROID
#include <jni.h>
#include "third_party/IPC/IPCResult.h"
#endif

namespace WeexCore {
    class WXCoreMargin;
    class WXCorePadding;
    class WXCoreBorderWidth;
    class WXCoreSize;
}  // namespace WeexCore
using namespace WeexCore;

class WeexJSResult{
public:
    std::unique_ptr<char[]> data;
    int length = 0;
    WeexJSResult() : data(nullptr), length(0) {}
    WeexJSResult(std::unique_ptr<char[]> d, int l) : data(std::move(d)), length(l) {}
};
#ifdef OS_ANDROID

typedef void (*FuncSetJSVersion)(const char *jsVersion);

typedef void (*FuncReportException)(const char *pageId, const char *func, const char *exception_string);
typedef void (*FuncReportServerCrash)(const char *instance_id);
typedef void (*FuncReportNativeInitStatus)(const char *status_code, const char *error_msg);

typedef void (*FuncCallNative)(const char *pageId, const char *task, const char *callback);

typedef std::unique_ptr<ValueWithType> (*FuncCallNativeModule)(const char *pageId, const char *module, const char *method,
                                                           const char *arguments, int argumentsLen, const char *options, int optionsLen);

typedef void (*FuncCallNativeComponent)(const char *pageId, const char *ref,
                                        const char *method, const char *arguments, int argumentsLen, const char *options, int optionsLen);

typedef void (*FuncCallAddElement)(const char *pageId, const char *parentRef, const char *domStr, int domLen,
                                   const char *index_cstr);

typedef void (*FuncSetTimeout)(const char *callbackId, const char *time);

typedef void (*FuncCallNativeLog)(const char *str_array);

typedef void (*FuncCallCreateBody)(const char *pageId, const char *domStr, int domStrLen);

typedef int (*FuncCallUpdateFinish)(const char *pageId, const char *task,int taskLen, const char *callback, int callbackLen);

typedef void (*FuncCallCreateFinish)(const char *pageId);

typedef int (*FuncCallRefreshFinish)(const char *pageId, const char *task, const char *callback);

typedef void (*FuncCallUpdateAttrs)(const char *pageId, const char *ref, const char *data, int dataLen);

typedef void (*FuncCallUpdateStyle)(const char *pageId, const char *ref, const char *data, int dataLen);

typedef void (*FuncCallRemoveElement)(const char *pageId, const char *ref);

typedef void (*FuncCallMoveElement)(const char *pageId, const char *ref, const char *parentRef, int index);

typedef void (*FuncCallAddEvent)(const char *pageId, const char *ref, const char *event);

typedef void (*FuncCallRemoveEvent)(const char *pageId, const char *ref, const char *event);

typedef int (*FuncSetInterval)(const char *pageId, const char *callbackId, const char *_time);

typedef void (*FuncClearInterval)(const char *pageId, const char *callbackId);

typedef const char *(*FuncCallGCanvasLinkNative)(const char *pageId, int type, const char *args);

typedef const char *(*FuncT3dLinkNative)(int type, const char *args);

typedef void (*FuncCallHandlePostMessage)(const char *vimId, const char *data, int dataLength);

typedef void
(*FuncCallDIspatchMessage)(const char *clientId, const char *data, int dataLength, const char *callback, const char *vmId);

typedef std::unique_ptr<WeexJSResult> (*FuncCallDispatchMessageSync)(
    const char *clientId, const char *data, int dataLength, const char *vmId);

typedef void
(*FuncOnReceivedResult)(long callback_id, std::unique_ptr<WeexJSResult>& result);
typedef void
(*FuncUpdateComponentData)(const char* page_id, const char* cid, const char* json_data);


typedef bool
(*FuncLog)(int level, const char *tag,
           const char *file,
           unsigned long line,
           const char *log);
typedef void
(*FuncCompileQuickJSBinCallback)(const char* key, const char* bytecode, int length);
typedef struct FunctionsExposedByCore {
    FuncSetJSVersion funcSetJSVersion;
    FuncReportException funcReportException;
    FuncCallNative funcCallNative;
    FuncCallNativeModule funcCallNativeModule;
    FuncCallNativeComponent funcCallNativeComponent;
    FuncCallAddElement funcCallAddElement;
    FuncSetTimeout funcSetTimeout;
    FuncCallNativeLog funcCallNativeLog;
    FuncCallCreateBody funcCallCreateBody;
    FuncCallUpdateFinish funcCallUpdateFinish;
    FuncCallCreateFinish funcCallCreateFinish;
    FuncCallRefreshFinish funcCallRefreshFinish;
    FuncCallUpdateAttrs funcCallUpdateAttrs;
    FuncCallUpdateStyle funcCallUpdateStyle;
    FuncCallRemoveElement funcCallRemoveElement;
    FuncCallMoveElement funcCallMoveElement;
    FuncCallAddEvent funcCallAddEvent;
    FuncCallRemoveEvent funcCallRemoveEvent;
    FuncSetInterval funcSetInterval;
    FuncClearInterval funcClearInterval;
    FuncCallGCanvasLinkNative funcCallGCanvasLinkNative;
    FuncT3dLinkNative funcT3dLinkNative;
    FuncCallHandlePostMessage funcCallHandlePostMessage;
    FuncCallDIspatchMessage funcCallDIspatchMessage;
    FuncCallDispatchMessageSync funcCallDispatchMessageSync;
    FuncOnReceivedResult  funcOnReceivedResult;
    FuncUpdateComponentData funcUpdateComponentData;
    FuncLog funcLog;
    FuncCompileQuickJSBinCallback funcCompileQuickJsBinCallback;
} FunctionsExposedByCore;



typedef int (*FuncInitFramework)(const char *script, std::vector<std::pair<std::string, std::string>>params);


typedef int (*FuncInitAppFramework)(const char *instanceId, const char *appFramework,
                                    std::vector<std::pair<std::string, std::string>>params);

typedef int (*FuncCreateAppContext)(const char *instanceId, const char *jsBundle);

typedef std::unique_ptr<WeexJSResult> (*FuncExeJSOnAppWithResult)(const char *instanceId, const char *jsBundle);

typedef int (*FuncCallJSOnAppContext)(const char *instanceId, const char *func, std::vector<VALUE_WITH_TYPE *> &params);

typedef int (*FuncDestroyAppContext)(const char *instanceId);

typedef int (*FuncExeJsService)(const char *source);

typedef int (*FuncExeCTimeCallback)(const char *source);

typedef int (*FuncExeJS)(const char *instanceId, const char *nameSpace, const char *func,
                         std::vector<VALUE_WITH_TYPE *> &params);

typedef std::unique_ptr<WeexJSResult>(*FuncExeJSWithResult)(const char *instanceId, const char *nameSpace, const char *func,
                                           std::vector<VALUE_WITH_TYPE *> &params);

typedef void(*FuncExeJSWithResultId)(const char *instanceId, const char *nameSpace, const char *func,
                                           std::vector<VALUE_WITH_TYPE *> &params, long callback_id);

typedef int (*FuncCreateInstance)(const char *instanceId, const char *func, const char *script, const int script_size, const char *opts,
                                  const char *initData, const char *extendsApi, std::vector<std::pair<std::string, std::string>> params);

typedef std::unique_ptr<WeexJSResult> (*FuncExeJSOnInstance)(const char *instanceId, const char *script,const int script_size, int type);

typedef int (*FuncDestroyInstance)(const char *instanceId);

typedef int (*FuncUpdateGlobalConfig)(const char *config);

typedef int (*FuncUpdateInitFrameworkParams)(const std::string& key, const std::string& value, const std::string& desc);

typedef void (*FuncSetLogType)(const int logLevel, const bool isPerf);
typedef void (*FuncCompileQuickJSBin)(const char *key,
                                      const char *script);
typedef struct FunctionsExposedByJS {
    FuncInitFramework funcInitFramework;
    FuncInitAppFramework funcInitAppFramework;
    FuncCreateAppContext funcCreateAppContext;
    FuncExeJSOnAppWithResult funcExeJSOnAppWithResult;
    FuncCallJSOnAppContext funcCallJSOnAppContext;
    FuncDestroyAppContext funcDestroyAppContext;
    FuncExeJsService funcExeJsService;
    FuncExeCTimeCallback funcExeCTimeCallback;
    FuncExeJS funcExeJS;
    FuncExeJSWithResult funcExeJSWithResult;
    FuncExeJSWithResultId funcExeJSWithResultId;
    FuncCreateInstance funcCreateInstance;
    FuncExeJSOnInstance funcExeJSOnInstance;
    FuncDestroyInstance funcDestroyInstance;
    FuncUpdateGlobalConfig funcUpdateGlobalConfig;
    FuncUpdateInitFrameworkParams funcUpdateInitFrameworkParams;
    FuncSetLogType funcSetLogType;
    FuncCompileQuickJSBin funcCompileQuickJSBin;
} FunctionsExposedByJS;

#endif

#endif //WEEXV8_WEEXAPIHEADER_H
