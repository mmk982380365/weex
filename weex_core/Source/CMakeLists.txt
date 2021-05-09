#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
cmake_minimum_required(VERSION 3.4.1)
set(WEEXCORE_LIBRARY_NAME_QJS weexcoreqjs)
set(WEEXCORE_LIBRARY_NAME weexcore)
set(WEEX_CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

message("${ANDROID_STL} ")
if ("${ANDROID_STL}" STREQUAL "c++_static" OR "${ANDROID_STL}" STREQUAL "c++_shared")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
elseif ("${ANDROID_STL}" STREQUAL "gnustl_static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1y")
endif ()
add_compile_options(-fexceptions)
add_compile_options(-fstack-protector)
add_compile_options(-funwind-tables)
add_compile_options(-fno-omit-frame-pointer)

add_definitions(-DGNU_SUPPORT=1)
add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT=1)

add_definitions(-DRENDER_LOG=0)
add_definitions(-DPERFORMANCE_LOG=0)
add_definitions(-DLAYOUT_LOG=0)
add_definitions(-DJSAPI_LOG=0)
add_definitions(-DDOM_PARSER_LOG=0)
#add_definitions(-DDEBUG=1)
add_definitions(-DNDEBUG=1)

message("ANDROID_PROJECT_DIR:" ${ANDROID_PROJECT_DIR})
set(LOCAL_LIBRARIES_DIR ${ANDROID_PROJECT_DIR}/libs/${ANDROID_ABI})

if ("${ANDROID_ABI}" STREQUAL "x86")
    # todo
else ()
    if ("${ENABLE_ASAN}" STREQUAL "true")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g3 -fPIC")
    endif ()
endif ()

add_compile_options(-fexceptions)

set(COMMON_SRCS
        ./core/render/manager/render_manager.cpp
        ./core/render/page/render_page.cpp
        ./core/render/page/render_page_base.cpp
        ./core/render/page/render_page_custom.cpp
        ./core/render/target/render_target.cpp

        ./core/render/node/render_object.cpp
        ./core/render/node/factory/render_creator.cpp
        ./core/render/node/render_appbar.cpp
        ./core/render/node/render_list.cpp
        ./core/render/node/render_mask.cpp
        ./core/render/node/render_scroller.cpp
        ./core/render/node/render_text.cpp

        ./core/render/action/render_action_add_event.cpp
        ./core/render/action/render_action_remove_event.cpp
        ./core/render/action/render_action_add_element.cpp
        ./core/render/action/render_action_remove_element.cpp
        ./core/render/action/render_action_move_element.cpp
        ./core/render/action/render_action_createbody.cpp
        ./core/render/action/render_action_createfinish.cpp
        ./core/render/action/render_action_appendtree_createfinish.cpp
        ./core/render/action/render_action_layout.cpp
        ./core/render/action/render_action_update_attr.cpp
        ./core/render/action/render_action_update_style.cpp
        ./core/render/action/render_action_render_success.cpp
        ./core/render/action/render_action_trigger_vsync.cpp
        ./core/json/JsonRenderManager.cpp
        ./core/json/JsonPage.cpp
        ./core/render/action/render_action_add_child_to_richtext.cpp
        ./core/render/action/render_action_remove_child_from_richtext.cpp
        ./core/render/action/render_action_update_richtext_child_style.cpp
        ./core/render/action/render_action_update_richtext_child_attr.cpp
        ./core/layout/layout.cpp
        ./core/layout/style.cpp

        ./core/css/css_value_getter.cpp

        ./core/config/core_environment.cpp

        ./core/moniter/render_performance.cpp
        ./core/bridge/platform/core_side_in_platform.cpp
        ./core/bridge/script/core_side_in_script.cpp
        ./core/parser/dom_wson.cpp
        ./core/parser/action_args_check.cpp

        ./core/network/http_module.cc
        )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/base)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/wson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/wson)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/json11)
#link_directories(${LOCAL_LIBRARIES_DIR})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/qjs/libs/${ANDROID_ABI})

SET(FINAL_ADD_LIBRARY
        ${COMMON_SRCS}
        )
SET(FINAL_TARGET_LINK_LIBRARIES wson json11)

if (ANDROID)
    add_definitions(-DOS_ANDROID=1)
    ## add_subdirectory for subdirectory has a CMakeLists.txt
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/IPC)
    # add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/android/jsengine)
    # add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/quickjs)
    ## include_directories for include head file
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/IPC)

#    set(PORT JSCOnly)
    add_definitions(-DPRINT_LOG_CACHEFILE=0)
    add_definitions(-DOS_ANDROID)
    #add_definitions(-DNDEBUG)
    add_definitions(-DNDEBUG=1)
    add_definitions(-DUSE_SYSTEM_MALLOC=0)
    add_definitions(-DWTF_OS_ANDROID=1)
    add_definitions(-DUSE_EXPORT_MACROS=1)
    if (${ANDROID_PLATFORM} STREQUAL "android-21")
    else ()
        add_definitions(-D_POSIX_THREAD_KEYS_MAX=128)
        add_definitions(-DPTHREAD_KEYS_MAX=_POSIX_THREAD_KEYS_MAX)
    endif ()

    if ("${ANDROID_ABI}" STREQUAL "armeabi")
        add_definitions(-DWTF_CPU_ARM)
        set(WTF_CPU_ARM 1)

        if ("${ANDROID_TOOLCHAIN}" STREQUAL clang)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a")
        endif ()
    elseif ("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
        add_definitions(-DWTF_CPU_ARM)
        set(WTF_CPU_ARM 1)
    elseif ("${ANDROID_ABI}" STREQUAL "arm64-v8a")
        add_definitions(-DWTF_CPU_ARM64)
        set(WTF_CPU_ARM64 1)
    else ()
        add_definitions(-DWTF_CPU_X86)
        set(WTF_CPU_X86_64 1)
    endif ()

    set(USE_SYSTEM_MALLOC 1)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

    include_directories(${WEEX_CORE_SOURCE_DIR})
    include_directories(${WEEX_CORE_SOURCE_DIR}/include)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/API)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/ForwardingHeaders)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/assembler)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/bytecode)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/dfg)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/disassembler)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/domjit)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/runtime)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/heap)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/jit)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/interpreter)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/parser)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/profiler)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/yarr)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/wtf/icu)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/wtf/text)
    include_directories(${WEEX_CORE_SOURCE_DIR}/include/wtf)


    set(ANDROID_SRCS_WITHOUT_JSC
            android/weex_extend_js_api.h
            android/weex_extend_js_api.cpp
            android/base/string/scoped_jstring_utf8.cpp
            android/base/string/jstring_cache.cpp
            android/base/string/scoped_jstring.cpp
            android/wrap/content_box_measurement_impl_android.cpp
            android/wrap/wx_debug_js_bridge.cpp
            android/wrap/native_render_object_utils_impl_android.cpp
            android/bridge/platform/android_side.cpp
            android/bridge/platform/android_bridge.cpp
            android/bridge/script/script_side_in_multi_process.cpp
            android/bridge/script/qjs/script_side_in_qjs.cpp
            android/bridge/script_bridge_in_multi_process.cpp
            android/bridge/script_bridge_in_simple.cpp
            core/bridge/script/script_side_in_simple.cpp

            android/bridge/multi_process_initializer.cpp
            android/wrap/wml_bridge.cpp
            android/utils/cache_utils.cpp
            android/utils/params_utils.cpp
            android/utils/so_utils.cpp
            android/utils/ipc_string_result.cpp
            android/wrap/log_utils.cpp
            android/wrap/hash_set.cpp
            android/wrap/wx_map.cpp
            android/wrap/wx_js_object.cpp
            android/wrap/wx_bridge.cpp
            android/wrap/wx_params.cpp
            android/multiprocess/weex_js_connection.cpp
            android/utils/jni_load.cc
            core/bridge/eagle_bridge.cpp
            core/bridge/eagle_bridge_ext.cpp
            core/network/android/default_request_handler.cc


            android/multiprocess/weex_ipc_client.h
            android/multiprocess/weex_ipc_client.cpp
            android/multiprocess/weex_ipc_server.h
            android/multiprocess/weex_ipc_server.cpp


            android/multiprocess/bridge/script/script_bridge_in_multi_process.cpp
            android/multiprocess/bridge/script/core_side_in_multi_process.cpp

            core/runtime/weex_runtime.h
            core/runtime/weex_context.h
            core/runtime/weex_context_holder.cpp
            core/runtime/weex_context_holder.h
            core/runtime/weex_runtime_manager.cpp
            core/runtime/weex_runtime_manager.h


            core/runtime/qjs/weex_runtime_qjs.h
            core/runtime/qjs/weex_runtime_qjs.cpp
            core/runtime/qjs/weex_context_qjs.h
            core/runtime/qjs/weex_context_qjs.cpp

            android/multiprocess/weex_progress_env.h
            android/multiprocess/weex_progress_env.cpp

            android/utils/wson_qjs.h
            android/utils/wson_qjs.cpp
            )

    set(ANDROID_SRCS_JSC
            android/jsengine/jsc/weex_runtime_jsc.h
            android/jsengine/jsc/weex_runtime_jsc.cpp
            android/jsengine/jsc/weex_utils_jsc.h
            android/jsengine/jsc/weex_utils_jsc.cpp
            android/jsengine/jsc/weex_global_object.h
            android/jsengine/jsc/weex_global_object.cpp
            android/jsengine/jsc/weex_context_jsc.h
            android/jsengine/jsc/weex_context_jsc.cpp
            android/jsengine/jsc/args.h
            android/jsengine/jsc/args.cpp
            android/jsengine/jsc/weex_console_object.h
            android/jsengine/jsc/weex_console_object.cpp
            android/jsengine/jsc/weex_simple_object.h
            android/jsengine/jsc/weex_simple_object.cpp
            android/utils/wson_jsc.h
            android/utils/wson_jsc.cpp
            )

    link_directories(${LOCAL_LIBRARIES_DIR})
    message("LOCAL_LIBRARIES_DIR:" ${LOCAL_LIBRARIES_DIR})
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "toolchain_exelinkflags" FORCE)
    SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -shared")

    SET(FINAL_ADD_LIBRARY_WITHOUTJSC
            ${FINAL_ADD_LIBRARY}
            ${ANDROID_SRCS_WITHOUT_JSC}
            )
    SET(FINAL_ADD_LIBRARY
            ${FINAL_ADD_LIBRARY_WITHOUTJSC}
            ${ANDROID_SRCS_JSC}
            )

    SET(FINAL_TARGET_LINK_LIBRARIES_WITHOUTJSC
            ${FINAL_TARGET_LINK_LIBRARIES}
            weexbase
            wson
            android
            weexipc
            quickjs
            WTF
            log
            -lc
            -ldl
            -pthread
            log
            c
            z
            )
    SET(FINAL_TARGET_LINK_LIBRARIES
    ${FINAL_TARGET_LINK_LIBRARIES_WITHOUTJSC}
    JavaScriptCore
    )

endif (ANDROID)

#if(iOS)
#add_definitions(-DTARGET_OS_IPHONE=1)
#endif(iOS)


add_library(${WEEXCORE_LIBRARY_NAME} SHARED ${FINAL_ADD_LIBRARY})
target_include_directories(${WEEXCORE_LIBRARY_NAME} PUBLIC .)
target_link_libraries(${WEEXCORE_LIBRARY_NAME} ${FINAL_TARGET_LINK_LIBRARIES})

add_library(${WEEXCORE_LIBRARY_NAME_QJS} SHARED ${FINAL_ADD_LIBRARY_WITHOUTJSC})
target_include_directories(${WEEXCORE_LIBRARY_NAME_QJS} PUBLIC .)
target_link_libraries(${WEEXCORE_LIBRARY_NAME_QJS} ${FINAL_TARGET_LINK_LIBRARIES_WITHOUTJSC})


# Enable it for smaller so.
# set_target_properties(${WEEXCORE_LIBRARY_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)

if (ANDROID)
    # Compile libweexjsb.so
    set(WEEXJSSERVERSTUB_NAME libweexjsb.so)
    add_executable(${WEEXJSSERVERSTUB_NAME}
            android/multiprocess/weex_js_server_main_stub.cpp)

    set_target_properties(${WEEXJSSERVERSTUB_NAME}
            PROPERTIES
            LINK_FLAGS
            "-pie -fPIE"
            )

    target_link_libraries(${WEEXJSSERVERSTUB_NAME}
            ${WEEXCORE_LIBRARY_NAME}
            )

    if (APPLE)
        set_target_properties(${WEEXJSSERVERSTUB_NAME}
                PROPERTIES LINK_FLAGS
                "-Wl,--entry=_start"
                )
    endif ()

    # Compile libweexjst.so
    set(WEEXJSSERVERSTUB_NAME_BELOW_L libweexjst.so)
    add_executable(${WEEXJSSERVERSTUB_NAME_BELOW_L}
            android/multiprocess/weex_js_server_main_stub.cpp)

    target_link_libraries(${WEEXJSSERVERSTUB_NAME_BELOW_L}
            ${WEEXCORE_LIBRARY_NAME}
            )

    if (APPLE)
        set_target_properties(${WEEXJSSERVERSTUB_NAME_BELOW_L}
                PROPERTIES LINK_FLAGS
                "-Wl,--entry=_start"
                )
    endif ()
endif (ANDROID)