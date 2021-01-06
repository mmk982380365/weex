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

#include "core_side_in_multi_process.h"
#include <unistd.h>
#include <memory>
#include <string>
#include <base/string_util.h>
#include <base/make_copyable.h>
#include <base/thread/waitable_event.h>
#include "android/multiprocess/weex_ipc_server.h"
#include "android/multiprocess/weex_progress_env.h"
#include "base/utils/log_utils.h"
#include "third_party/IPC/Buffering/IPCBuffer.h"
#include "third_party/IPC/IPCException.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/IPCSender.h"
#include "third_party/IPC/IPCType.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"
#include "third_party/IPC/IPCMessageJS.h"

namespace weex {
namespace bridge {
namespace js {

CoreSideInMultiProcess::CoreSideInMultiProcess() {
  ipc_msg_thread_ = std::make_unique<weex::base::Thread>(weex::base::MessageLoop::DEFAULT);
  ipc_msg_thread_->Start();
  LOGE("thread isStarted and IPC %u", pthread_self());
}

CoreSideInMultiProcess::~CoreSideInMultiProcess() = default;

void CoreSideInMultiProcess::CallNative(const char *page_id, const char *task,
                                        const char *callback) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVE));
  serializer->add(page_id, strlen(page_id));
  serializer->add(task, strlen(task));
  serializer->add(callback, strlen(callback));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

std::unique_ptr<ValueWithType> CoreSideInMultiProcess::CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVEMODULE));
  serializer->add(page_id, strlen(page_id));
  serializer->add(module, strlen(module));
  serializer->add(method, strlen(method));
  serializer->add(arguments, arguments_length);
  serializer->add(options, options_length);

  weex::base::WaitableEvent event;
  std::unique_ptr<IPCResult> ipc_result;
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer), e = &event, ret = &ipc_result] {
    if (this->ipc_client_ != nullptr)
      *ret = this->ipc_client_->getSender()->send(msg->finish().get());
    e->Signal();
  }));
  event.Wait();
  std::unique_ptr<ValueWithType> ret(new ValueWithType);

  switch (ipc_result->getType()) {
    case IPCType::DOUBLE: {
      ret->type = ParamsType::DOUBLE;
      ret->value.doubleValue = ipc_result->get<double>();
      break;
    }
    case IPCType::STRING: {
      ret->type = ParamsType::STRING;
      ret->value.string = weex::base::genWeexStringSS(ipc_result->getStringContent(),
                                                      ipc_result->getStringLength());
      break;
    }
    case IPCType::JSONSTRING: {
      ret->type = ParamsType::JSONSTRING;
      ret->value.string = weex::base::genWeexStringSS(ipc_result->getStringContent(),
                                                      ipc_result->getStringLength());
      break;
    }
    case IPCType::BYTEARRAY: {
      ret->type = ParamsType::BYTEARRAY;
      ret->value.byteArray = weex::base::genWeexByteArraySS(ipc_result->getByteArrayContent(),
                                                            ipc_result->getByteArrayLength());
    }
      break;
    default:ret->type = ParamsType::JSUNDEFINED;
  }
  return ret;
}

void CoreSideInMultiProcess::CallNativeComponent(
    const char *page_id, const char *ref, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVECOMPONENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(method, strlen(method));
  serializer->add(arguments, arguments_length);
  serializer->add(options, options_length);
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::AddElement(const char *page_id,
                                        const char *parent_ref,
                                        const char *dom_str, int dom_str_length,
                                        const char *index_str) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLADDELEMENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(parent_ref, strlen(parent_ref));
  serializer->add(dom_str, dom_str_length);
  serializer->add(index_str, strlen(index_str));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::SetTimeout(const char *callback_id,
                                        const char *time) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::SETTIMEOUT));
  serializer->add(callback_id, strlen(callback_id));
  serializer->add(time, strlen(time));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::NativeLog(const char *str_array) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::NATIVELOG));
  serializer->add(str_array, strlen(str_array));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::CreateBody(const char *page_id,
                                        const char *dom_str,
                                        int dom_str_length) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEBODY));
  serializer->add(page_id, strlen(page_id));
  serializer->add(dom_str, dom_str_length);
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

int CoreSideInMultiProcess::UpdateFinish(const char *page_id, const char *task,
                                         int task_length, const char *callback,
                                         int callback_length) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEFINISH));
  serializer->add(page_id, strlen(page_id));
  serializer->add(task, task_length);
  serializer->add(callback, callback_length);

  std::unique_ptr<IPCResult> ipc_result;
  weex::base::WaitableEvent event;
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer), ret = &ipc_result, e = &event] {
    if (this->ipc_client_ != nullptr)
      *ret = this->ipc_client_->getSender()->send(msg->finish().get());
    e->Signal();
  }));
  event.Wait();
  if (ipc_result->getType() != IPCType::INT32) {
    LOGE("functionCallUpdateFinish: unexpected result: %d", ipc_result->getType());
    return 0;
  }
  return ipc_result->get<int32_t>();
}

void CoreSideInMultiProcess::CreateFinish(const char *page_id) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEFINISH));
  serializer->add(page_id, strlen(page_id));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

int CoreSideInMultiProcess::RefreshFinish(const char *page_id, const char *task,
                                          const char *callback) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLREFRESHFINISH));
  serializer->add(page_id, strlen(page_id));
  serializer->add(task, strlen(task));
  serializer->add(callback, strlen(callback));
  std::unique_ptr<IPCResult> ipc_result;
  weex::base::WaitableEvent event;
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer), ret = &ipc_result, e = &event] {
    if (this->ipc_client_ != nullptr)
      *ret = this->ipc_client_->getSender()->send(msg->finish().get());
    e->Signal();
  }));
  event.Wait();

  if (ipc_result->getType() != IPCType::INT32) {
    LOGE("functionCallRefreshFinish: unexpected result: %d", ipc_result->getType());
    return 0;
  }
  return ipc_result->get<int32_t>();
}

void CoreSideInMultiProcess::UpdateAttrs(const char *page_id, const char *ref,
                                         const char *data, int data_length) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEATTRS));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(data, data_length);
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::UpdateStyle(const char *page_id, const char *ref,
                                         const char *data, int data_length) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATESTYLE));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(data, data_length);
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::RemoveElement(const char *page_id,
                                           const char *ref) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEELEMENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::MoveElement(const char *page_id, const char *ref,
                                         const char *parent_ref, int index) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLMOVEELEMENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(parent_ref, strlen(parent_ref));
  serializer->add(index);
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::AddEvent(const char *page_id, const char *ref,
                                      const char *event) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLADDEVENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(event, strlen(event));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::RemoveEvent(const char *page_id, const char *ref,
                                         const char *event) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEEVENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(event, strlen(event));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

const char *CoreSideInMultiProcess::CallGCanvasLinkNative(
    const char *context_id, int type, const char *arg) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLGCANVASLINK));
  serializer->add(context_id, strlen(context_id));
  serializer->add(type);
  serializer->add(arg, strlen(arg));

  weex::base::WaitableEvent event;
  std::unique_ptr<IPCResult> result;
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer), ret = &result, e = &event] {
    if (this->ipc_client_ != nullptr)
      *ret = this->ipc_client_->getSender()->send(msg->finish().get());
    e->Signal();
  }));
  event.Wait();
  try {
    if (result->getType() != IPCType::VOID) {
      if (result->getStringLength() > 0) {
        return weex::base::to_utf8(const_cast<uint16_t *>(result->getStringContent()),
                                   result->getStringLength()).c_str();
      }
    }
  } catch (IPCException &e) {
    LOGE("functionGCanvasLinkNative exception: %s", e.msg());
    _exit(1);
  }
  return NULL;
}

int CoreSideInMultiProcess::SetInterval(const char *page_id,
                                        const char *callback_id,
                                        const char *time) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::SETINTERVAL));
  serializer->add(page_id, strlen(page_id));
  serializer->add(callback_id, strlen(callback_id));
  serializer->add(time, strlen(time));
  weex::base::WaitableEvent event;
  std::unique_ptr<IPCResult> result;
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer), ret = &result, e = &event] {
    if (this->ipc_client_ != nullptr)
      *ret = this->ipc_client_->getSender()->send(msg->finish().get());
    e->Signal();
  }));
  event.Wait();
  if (result->getType() != IPCType::INT32) {
    LOGE("functionSetIntervalWeex: unexpected result: %d", result->getType());
    return 0;
  }
  return result->get<int32_t>();
}

void CoreSideInMultiProcess::ClearInterval(const char *page_id,
                                           const char *callback_id) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CLEARINTERVAL));
  serializer->add(page_id, strlen(page_id));
  serializer->add(callback_id, strlen(callback_id));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

const char *CoreSideInMultiProcess::CallT3DLinkNative(int type,
                                                      const char *arg) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLT3DLINK));
  serializer->add(type);
  serializer->add(arg, strlen(arg));
  weex::base::WaitableEvent event;
  std::unique_ptr<IPCResult> result;
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer), ret = &result, e = &event] {
    if (this->ipc_client_ != nullptr)
      *ret = this->ipc_client_->getSender()->send(msg->finish().get());
    e->Signal();
  }));
  event.Wait();
  if (result->getType() != IPCType::VOID) {
    if (result->getStringLength() > 0) {
      return weex::base::to_utf8(const_cast<uint16_t *>(result->getStringContent()),
                                 result->getStringLength()).c_str();
    }
  }
  return nullptr;
}

void CoreSideInMultiProcess::PostMessage(const char *vim_id, const char *data,
                                         int dataLength) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::POSTMESSAGE));
  serializer->add(data, dataLength);
  serializer->add(vim_id, strlen(vim_id));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::DispatchMessage(const char *client_id,
                                             const char *data,
                                             int dataLength,
                                             const char *callback,
                                             const char *vm_id) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::DISPATCHMESSAGE));
  serializer->add(client_id, strlen(client_id));
  serializer->add(data, dataLength);
  serializer->add(callback, strlen(callback));
  serializer->add(vm_id, strlen(vm_id));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

std::unique_ptr<WeexJSResult>
CoreSideInMultiProcess::DispatchMessageSync(const char *client_id,
                                            const char *data,
                                            int dataLength,
                                            const char *vm_id) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::DISPATCHMESSAGESYNC));
  serializer->add(client_id, strlen(client_id));
  serializer->add(data, dataLength);
  serializer->add(vm_id, strlen(vm_id));
  weex::base::WaitableEvent event;
  std::unique_ptr<IPCResult> result;
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer), ret = &result, e = &event] {
    if (this->ipc_client_ != nullptr)
      *ret = this->ipc_client_->getSender()->send(msg->finish().get());
    e->Signal();
  }));
  event.Wait();
  char *copy = nullptr;
  int length = 0;
  if (result->getType() == IPCType::BYTEARRAY) {
    length = result->getByteArrayLength();
    copy = new char[length];
    strcpy(copy, result->getByteArrayContent());
  }
  return std::make_unique<WeexJSResult>(std::unique_ptr<char[]>(copy), length);
}

void CoreSideInMultiProcess::ReportException(const char *page_id,
                                             const char *func,
                                             const char *exception_string) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::REPORTEXCEPTION));
  serializer->add(page_id, strlen(page_id));
  serializer->add(func, strlen(func));
  serializer->add(exception_string, strlen(exception_string));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::SetJSVersion(const char *js_version) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::SETJSFVERSION));
  serializer->add(js_version, strlen(js_version));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::OnReceivedResult(long callback_id,
                                              std::unique_ptr<WeexJSResult> &result) {

  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::ONRECEIVEDRESULT));

  auto temp = std::to_string(callback_id);
  serializer->add(temp.c_str(), temp.length());
  if (result != nullptr) {
    serializer->add(result->data.get(), result->length);
  }
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::UpdateComponentData(const char *page_id, const char *cid,
                                                 const char *json_data) {

  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::UPDATECOMPONENTDATA));

  serializer->add(page_id, strlen(page_id));
  serializer->add(cid, strlen(cid));
  serializer->add(json_data, strlen(json_data));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

bool CoreSideInMultiProcess::Log(int level, const char *tag,
                                 const char *file,
                                 unsigned long line,
                                 const char *log) {

  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::POSTLOGDETAIL));
  serializer->add(level);
  serializer->add(tag, strlen(tag));
  serializer->add(file, strlen(file));
  serializer->add((int64_t) line);
  serializer->add(log, strlen(log));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
  return true;
}
void CoreSideInMultiProcess::CompileQuickJSCallback(const char *key,
                                                    const char *bytecode,
                                                    int length) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::COMPILEQUICKJSBINCALLBACK));
  serializer->add(key, strlen(key));
  serializer->add(bytecode, length);
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

void CoreSideInMultiProcess::InitIpcClient(int clientFd) {
  ipc_msg_thread_->message_loop()->PostTask([=, fd = clientFd] {
    this->ipc_client_ = std::make_unique<WeexIPCClient>(fd);
  });
}
void CoreSideInMultiProcess::HeartBeat(const char *instanceId) {
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::HEARTBEAT));
  serializer->add(instanceId, strlen(instanceId));
  ipc_msg_thread_->message_loop()->PostTask(weex::base::MakeCopyable([=, msg =
  std::move(serializer)] {
    if (this->ipc_client_ != nullptr)
      this->ipc_client_->getSender()->send(msg->finish().get());
  }));
}

}  // namespace js
}  // namespace bridge
}  // namespace weex
