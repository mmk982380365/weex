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
package com.taobao.weex;

import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.bridge.WXReactorPage;
import com.taobao.weex.bridge.WXReactorPlugin;
import com.taobao.weex.utils.WXSoInstallMgrSdk;

public class WXReactorPluginManager {
    private static WXReactorPluginManager instance;
    private volatile WXReactorPlugin plugin;

    private WXReactorPluginManager() {
    }

    public static WXReactorPluginManager getInstance() {
        if (instance == null) {
            synchronized (WXReactorPluginManager.class) {
                if (instance == null) {
                    instance = new WXReactorPluginManager();
                }
            }
        }
        return instance;
    }

    public void init(WXReactorPlugin plugin) {
        if (this.plugin != null) {
            return;
        }
        this.plugin = plugin;
    }

    public void initSo(int version, IWXUserTrackAdapter utAdapter) {
        if (plugin != null) {
//            WXSoInstallMgrSdk.initSo(plugin.getSoLibName(), version, utAdapter);
        }
    }

    public WXReactorPage createPage(long v8_isolate, String instanceId) {
        if (plugin == null) {
            return null;
        }
        return plugin.createPage(v8_isolate, instanceId);
    }
}
