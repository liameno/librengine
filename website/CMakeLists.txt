cmake_minimum_required(VERSION 3.10)
project(website LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#lexbor - html, css / https://github.com/lexbor/lexbor
#curl - http lib / https://github.com/curl/curl
#openssl - encryption lib / https://www.openssl.org/

add_executable(${PROJECT_NAME} main.cpp third_party/httplib.h src/pages.cpp include/pages.h)

if (NOT TARGET librengine)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE librengine)