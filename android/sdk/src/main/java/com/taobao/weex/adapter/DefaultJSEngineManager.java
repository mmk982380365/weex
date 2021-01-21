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
package com.taobao.weex.adapter;

import android.util.Pair;

import com.taobao.weex.WXSDKEngine;

public class DefaultJSEngineManager implements IWXJSEngineManager {

    public boolean isEngineOn(EngineType engineType) {
        if (engineType == null) {
            return false;
        }

        if (engineType != defaultEngine()) {
            return engineType.engineOn();
        }

        if (engineType.engineOn()) {
            return true;
        }

        //如果所有的 ENGINE 都关, 那么 defaultEngine 需要开启.
        boolean allOff = true;
        for (EngineType t : EngineType.values()) {
            if (t.engineOn()) {
                allOff = false;
                break;
            }
        }

        return allOff;
    }

    @Override
    public EngineType defaultEngine() {
        return WXSDKEngine.defaultEngineType();
    }

    @Override
    public void setEngineSwitchValue(EngineType engineType, boolean value) {
        if (engineType != null) {
            engineType.setEngineValue(value);
        }
    }

    @Override
    public Pair<EngineType, String> engineType(String url) {
        return new Pair<>(defaultEngine(), url);
    }

    @Override
    public boolean runInMainProcess(String url) {
        return false;
    }

    @Override
    public void updateEnableUrlData(String data) {

    }

    @Override
    public void updateDisableUrlData(String data) {

    }

    @Override
    public boolean enableMainProcessScriptSide() {
        return true;
    }

    @Override
    public boolean forceAllPageRunInMainProcessScriptSide() {
        return false;
    }
}
