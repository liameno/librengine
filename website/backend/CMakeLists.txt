cmake_minimum_required(VERSION 3.20)
project(backend)

set(CMAKE_CXX_STANDARD 17)

#lexbor - html, css / https://github.com/lexbor/lexbor
#curl - http lib / https://github.com/curl/curl

find_package(CURL)
find_package(Threads)

add_executable(${PROJECT_NAME} main.cpp third_party/httplib.h)
target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/liblexbor.so curl Threads::Threads /usr/local/lib/liblibrengine.so)