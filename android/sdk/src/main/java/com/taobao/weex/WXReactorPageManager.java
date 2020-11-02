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

import android.os.Handler;
import android.os.Looper;

import com.taobao.weex.bridge.WXReactorPage;

import java.util.Map;

public class WXReactorPageManager {
    private WXReactorPage mReactorPage;
    private Handler mJsHandler;
    private String mAppId;

    public WXReactorPageManager(WXReactorPage reactorPage,Handler handler){
        this.mReactorPage = reactorPage;
        this.mJsHandler = handler;
    }
    public WXReactorPageManager(WXReactorPage reactorPage,Handler handler,String mAppId){
        this.mReactorPage = reactorPage;
        this.mJsHandler = handler;
        this.mAppId = mAppId;
    }

    public void post(Runnable r){
        if(mJsHandler == null){
            return;
        }
        if(mJsHandler.getLooper() == Looper.myLooper()){
            r.run();
        }
        else {
            mJsHandler.post(r);
        }
    }
    public Handler getJsHandler(){
        return mJsHandler;
    }
    public String getAppId(){
        return mAppId;
    }

    public void setPageContext(final long page_context){
        if(mReactorPage != null){
            post(new Runnable() {
                @Override
                public void run() {
                    mReactorPage.setPageContext(page_context);
                }
            });
        }
    }

    public void unregisterJSContext(){
        if(mReactorPage != null){
            post(new Runnable() {
                @Override
                public void run() {
                    mReactorPage.unregisterJSContext();
                }
            });
        }
    }

    public void render(final String script, final String initData){
        if(mReactorPage != null){
            post(new Runnable() {
                @Override
                public void run() {
                    mReactorPage.render(script,initData);
                }
            });
        }
    }

    public void registerComponent(){
        if(mReactorPage != null){
            post(new Runnable() {
                @Override
                public void run() {
                    mReactorPage.registerComponent();
                }
            });
        }
    }

    public void invokeCallBack(final String callbackId,final String args){
        if(mReactorPage != null){
            post(new Runnable() {
                @Override
                public void run() {
                    mReactorPage.invokeCallBack(callbackId, args);
                }
            });
        }
    }

    public void fireEvent(final String ref, final String event, final Map<String, String> args, final String domChanges){
        if(mReactorPage != null){
            post(new Runnable() {
                @Override
                public void run() {
                    mReactorPage.fireEvent(ref,event,args, domChanges);
                }
            });
        }
    }

}
