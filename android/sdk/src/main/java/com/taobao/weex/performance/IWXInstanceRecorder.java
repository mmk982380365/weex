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
package com.taobao.weex.performance;


import com.alibaba.fastjson.JSONObject;

/**
 * 监听实例信息
 */
public interface IWXInstanceRecorder {
    // 类型
    enum RecordType {
        MtopRequest,
        ModuleRequest,
        NativeModuleInvoke,
        TemplateRequest
    }

    /**
     * 监听信息
     * @param info 信息内容
     * @param type 信息类型
     */
    void record(JSONObject info, RecordType type);

    /**
     * 监听信息
     */
    void record(String key, Object value);

    /**
     * 是否需要监听
     */
    boolean needRecord();

    /**
     * 上报监控信息
     */
    void uploadRecord();

    /**
     * 更新url
     */
    void updateUrl(String url);
}
