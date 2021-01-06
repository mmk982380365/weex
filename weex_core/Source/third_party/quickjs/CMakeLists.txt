cmake_minimum_required(VERSION 3.4.1)

set(QUICKJS_NAME quickjs)

add_definitions(-DCONFIG_VERSION="2020-04-12")
#add_definitions(-DCONFIG_BIGNUM=y)
#add_definitions(-DCONFIG_ALL_UNICODE=1)


add_library(${QUICKJS_NAME} SHARED
        ./cutils.c
        ./quickjs-libc.c
        ./quickjs.c
        ./libregexp.c
        ./libunicode.c
        )

target_include_directories(${QUICKJS_NAME} PUBLIC .)

#for log debug
target_link_libraries(${QUICKJS_NAME} log)