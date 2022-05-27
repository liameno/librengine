cmake_minimum_required(VERSION 3.20)
project(crawler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#lexbor - html, css / https://github.com/lexbor/lexbor
#curl - http lib / https://github.com/curl/curl

find_package(CURL)
find_package(Threads)

add_executable(${PROJECT_NAME} main.cpp src/worker.cpp include/worker.h src/json_generator.cpp include/json_generator.h src/html_helper.cpp include/html_helper.h ${tp_rep_cpp} ${tp_url_cpp})
target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/liblexbor.so curl Threads::Threads /usr/local/lib/liblibrengine.so)