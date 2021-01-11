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

#include "android/utils/so_utils.h"
#include <string>
#include <errno.h>
#include <cstdlib>
#include <unistd.h>
#include <dlfcn.h>
#include "base/android/log_utils.h"

namespace WeexCore {
    char *SoUtils::g_cache_dir = nullptr;
    char *SoUtils::g_crash_file_path = nullptr;
    char *SoUtils::g_jss_icu_path = nullptr;
    char *SoUtils::g_lib_ld_path = nullptr;
    char *SoUtils::g_jsb_so_cache_path = nullptr;
    char *SoUtils::g_jsb_so_inner_path = nullptr;
    bool SoUtils::g_pie_support = false;
    int SoUtils::g_android_api = 0;
    std::function<void(const char *, const char *)> SoUtils::g_exception_handler = nullptr;

    const char *SoUtils::GetDefaultCacheDir(JNIEnv *env) {
        jclass activityThreadCls, applicationCls, fileCls;
        jobject applicationObj, fileObj, pathStringObj;
        jmethodID currentApplicationMethodId,
                getCacheDirMethodId,
                getAbsolutePathMethodId;
        static std::string storage;
        const char *tmp;
        const char *ret = nullptr;
        if (!storage.empty()) {
            ret = storage.c_str();
            goto no_empty;
        }
        activityThreadCls = env->FindClass("android/app/ActivityThread");
        if (!activityThreadCls || env->ExceptionOccurred()) {
            goto no_class;
        }
        currentApplicationMethodId = env->GetStaticMethodID(
                activityThreadCls,
                "currentApplication",
                "()Landroid/app/Application;");
        if (!currentApplicationMethodId || env->ExceptionOccurred()) {
            goto no_currentapplication_method;
        }
        applicationObj = env->CallStaticObjectMethod(activityThreadCls,
                                                     currentApplicationMethodId,
                                                     nullptr);
        if (!applicationObj || env->ExceptionOccurred()) {
            goto no_application;
        }
        applicationCls = env->GetObjectClass(applicationObj);
        getCacheDirMethodId = env->GetMethodID(applicationCls,
                                               "getCacheDir",
                                               "()Ljava/io/File;");
        if (!getCacheDirMethodId || env->ExceptionOccurred()) {
            goto no_getcachedir_method;
        }
        fileObj = env->CallObjectMethod(applicationObj, getCacheDirMethodId, nullptr);
        if (!fileObj || env->ExceptionOccurred()) {
            goto no_file_obj;
        }
        fileCls = env->GetObjectClass(fileObj);
        getAbsolutePathMethodId = env->GetMethodID(fileCls,
                                                   "getAbsolutePath",
                                                   "()Ljava/lang/String;");
        if (!getAbsolutePathMethodId || env->ExceptionOccurred()) {
            goto no_getabsolutepath_method;
        }
        pathStringObj = env->CallObjectMethod(fileObj,
                                              getAbsolutePathMethodId,
                                              nullptr);
        if (!pathStringObj || env->ExceptionOccurred()) {
            goto no_path_string;
        }
        tmp = env->GetStringUTFChars(reinterpret_cast<jstring>(pathStringObj),
                                     nullptr);
        storage.assign(tmp);
        env->ReleaseStringUTFChars(reinterpret_cast<jstring>(pathStringObj),
                                   tmp);
        ret = storage.c_str();
        no_path_string:
        no_getabsolutepath_method:
        env->DeleteLocalRef(fileCls);
        env->DeleteLocalRef(fileObj);
        no_file_obj:
        no_getcachedir_method:
        env->DeleteLocalRef(applicationCls);
        env->DeleteLocalRef(applicationObj);
        no_application:
        no_currentapplication_method:
        env->DeleteLocalRef(activityThreadCls);
        no_class:
        env->ExceptionDescribe();
        env->ExceptionClear();
        no_empty:
        return ret;
    }

    void SoUtils::Init(JNIEnv *env) {
        g_cache_dir = const_cast<char *>(GetDefaultCacheDir(env));
    }


    void SoUtils::RegisterExceptionHanler(
            const std::function<void(const char*, const char*)>&
            ReportNativeInitStatus) {
        g_exception_handler = ReportNativeInitStatus;
    }

    void SoUtils::updateSoLinkPath(const char *lib_ld_path) {
        if(lib_ld_path == nullptr || strlen(lib_ld_path) == 0) {
          return;
        }

        LOGE("updateSoLinkPath is %s",lib_ld_path);
        void* sym = dlsym(RTLD_DEFAULT, "android_update_LD_LIBRARY_PATH");
        if (sym != NULL) {
            typedef void (*Fn)(const char*);
            Fn android_update_LD_LIBRARY_PATH = reinterpret_cast<Fn>(sym);
            (*android_update_LD_LIBRARY_PATH)(lib_ld_path);
        } else {
            LOGE("android_update_LD_LIBRARY_PATH not found; .so dependencies will not work!");
        }
    }
}
