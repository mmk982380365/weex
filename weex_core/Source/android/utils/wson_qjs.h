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
// Created by 董亚运 on 2020/8/11.
//

#ifndef WEEX_PROJECT_WSON_QJS_H
#define WEEX_PROJECT_WSON_QJS_H
#include <string>
#include "wson/wson.h"
extern "C" {
#include "include/qjs/quickjs.h"
}

wson_buffer *toWsonBuffer(JSContext *ctx, JSValue value);
void pushMapKeyToBuffer(wson_buffer *buffer, const std::string &str_utf_8);
void putValuesToWson(JSContext *ctx, JSValue value,
                     wson_buffer *buffer);
void pushStringToWsonBuffer(wson_buffer *buffer, const std::string &str_utf_8);
#endif //WEEX_PROJECT_WSON_QJS_H
