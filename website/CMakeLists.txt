cmake_minimum_required(VERSION 3.20)
project(website LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#lexbor - html, css / https://github.com/lexbor/lexbor
#curl - http lib / https://github.com/curl/curl
#openssl - encryption lib / https://www.openssl.org/

find_package(CURL)
find_package(Threads)
find_package(OpenSSL)

add_executable(${PROJECT_NAME} main.cpp third_party/httplib.h src/pages.cpp include/pages.h)
target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/liblexbor.so curl OpenSSL::Crypto Threads::Threads /usr/local/lib/liblibrengine.so)