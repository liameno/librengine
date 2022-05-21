cmake_minimum_required(VERSION 3.20)
project(crawler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#lexbor - html, css / https://github.com/lexbor/lexbor
#curl - http lib / https://github.com/curl/curl

find_package(CURL)
find_package(Threads)

set(tp_rep_cpp third_party/rep-cpp/agent.cpp third_party/rep-cpp/agent.h
        third_party/rep-cpp/directive.cpp third_party/rep-cpp/directive.h third_party/rep-cpp/robots.cpp
        third_party/rep-cpp/robots.h) #robots.txt / https://github.com/seomoz/rep-cpp
set(tp_url_cpp third_party/url-cpp/psl.cpp third_party/url-cpp/psl.h
        third_party/url-cpp/punycode.cpp third_party/url-cpp/punycode.h third_party/url-cpp/url.cpp
        third_party/url-cpp/url.h third_party/url-cpp/utf8.cpp third_party/url-cpp/utf8.h) #https://github.com/seomoz/url-cpp

add_executable(${PROJECT_NAME} main.cpp src/worker.cpp include/worker.h ${tp_rep_cpp} ${tp_url_cpp})
target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/liblexbor.so curl Threads::Threads /usr/local/lib/liblibrengine.so)