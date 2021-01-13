/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * <p>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p>
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.adapter;

import android.text.TextUtils;
import android.util.Pair;

import java.util.concurrent.CopyOnWriteArrayList;

public interface IWXJSEngineManager {
    EngineType defaultEngine();

    void setEngineSwitchValue(EngineType engineType, boolean value);

    // url -> data
    Pair<EngineType, String> engineType(String url);

    //Online Config -> url : engineType
    void updateEnableUrlData(String data);

    void updateDisableUrlData(String data);

    boolean enableMainProcessScriptSide();

    boolean forceAllPageRunInMainProcessScriptSide();

    public enum EngineType {
        JavaScriptCore("JSC", true, 1),
        QuickJSBin("QJSBin", false, 2),
        QuickJS("QJS", false, 4 | QuickJSBin.engineValue);
        private boolean switchValue;
        private int engineValue;
        private String engineName;
        private CopyOnWriteArrayList<String> urlList = new CopyOnWriteArrayList<>();

        EngineType(String engineName, boolean switchValue, int engineValue) {
            this.switchValue = switchValue;
            this.engineName = engineName;
            this.engineValue = engineValue;
        }

        public boolean engineOn() {
            return switchValue;
        }

        public String engineName() {
            return engineName;
        }

        public int engineValue() {
            return engineValue;
        }

        public void setEngineValue(boolean value) {
            this.switchValue = value;
        }

        public boolean hasPage(String url) {
            if (TextUtils.isEmpty(url)) {
                return false;
            }
            return urlList.contains(url);
        }
    }
}
