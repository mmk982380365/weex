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

#include "base/string_util.h"
#include "wson/wson_parser.h"
#include "wson_qjs.h"
wson_buffer *toWsonBuffer(JSContext *ctx, JSValue value) {
  // const char *result = JS_ToCString(ctx, JS_ToString(ctx, value));
  wson_buffer *buffer = wson_buffer_new();
  putValuesToWson(ctx, value, buffer);
  wson_parser parser((char *) buffer->data);
//  const std::string &string = parser.toStringUTF8();
  return buffer;
}
// add for wson
void pushStringToWsonBuffer(wson_buffer *buffer, const std::string &str_utf_8) {
  std::u16string s = weex::base::to_utf16(const_cast<char *>(str_utf_8.c_str()),
                                          str_utf_8.length());
  size_t length = s.length();
  wson_push_type(buffer, WSON_STRING_TYPE);
  wson_push_uint(buffer, length * sizeof(uint16_t));
  wson_push_bytes(buffer, s.c_str(), s.length() * sizeof(uint16_t));
}

void pushMapKeyToBuffer(wson_buffer *buffer, const std::string &str_utf_8) {
  std::u16string s = weex::base::to_utf16(const_cast<char *>(str_utf_8.c_str()),
                                          str_utf_8.length());
  size_t length = s.length();
  wson_push_uint(buffer, length * sizeof(uint16_t));
  wson_push_bytes(buffer, s.c_str(), s.length() * sizeof(uint16_t));
}

void putValuesToWson(JSContext *ctx, JSValue value,
                     wson_buffer *buffer) {
  int tag = JS_VALUE_GET_TAG(value);

  if (tag == JS_TAG_NULL || tag == JS_TAG_UNDEFINED) {
    wson_push_type_null(buffer);
    return;
  }

  switch (tag) {
    case JS_TAG_INT: {
      int num = -1;
      JS_ToInt32(ctx, &num, value);
      wson_push_type_int(buffer, num);
      break;
    }
    case JS_TAG_BOOL: {
      wson_push_type_boolean(buffer, JS_ToBool(ctx, value));
      break;
    }
    case JS_TAG_FLOAT64: {
      double numd;
      JS_ToFloat64(ctx, &numd, value);
      wson_push_type_double(buffer, numd);
      break;
    }
    case JS_TAG_UNDEFINED: {
      wson_push_type_null(buffer);
      break;
    }
    case JS_TAG_STRING: {
      const char *str = JS_ToCString(ctx, value);
      pushStringToWsonBuffer(buffer, str);
      break;
    }
    case JS_TAG_OBJECT: {
      if (JS_IsArray(ctx, value)) {
        JSValue length = JS_GetPropertyStr(ctx, value, "length");
        int len = JS_VALUE_GET_INT(length);
        JS_FreeValue(ctx, length);
        wson_push_type_array(buffer, len);
        for (size_t i = 0; i < len; i++) {
          JSValue item = JS_GetPropertyUint32(ctx, value, i);
          putValuesToWson(ctx, item, buffer);
        }
      } else {
        JSPropertyEnum *tab_atom;
        uint32_t tab_atom_count;
        JS_GetOwnPropertyNames(
            ctx, &tab_atom, &tab_atom_count, value,
            JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY);
        wson_push_type_map(buffer, tab_atom_count);
        for (size_t i = 0; i < tab_atom_count; i++) {
          JSAtom atom = tab_atom[i].atom;
          JSValue item = JS_GetProperty(ctx, value, atom);
          JSValue key = JS_AtomToValue(ctx, atom);

          const char *str = JS_ToCString(ctx, key);
          pushMapKeyToBuffer(buffer, str);
          putValuesToWson(ctx, item, buffer);
        }
      }
      /* code */
      break;
    }
    default: {
      if (JS_TAG_IS_FLOAT64(tag)) {
        double numd;
        JS_ToFloat64(ctx, &numd, value);
        wson_push_type_double(buffer, numd);
      }
      break;
    }
  }
}