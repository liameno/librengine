cmake_minimum_required(VERSION 3.10)
project(librengine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(include include/encryption.h include/typesense.h include/http.h include/str.h include/str_impl.h include/config.h include/logger.h include/json.hpp include/structs.h include/search.h include/helper.h include/cache.h include/robots_txt.h)
set(src src/encryption.cpp src/typesense.cpp src/http.cpp src/logger.cpp src/config.cpp src/search.cpp src/helper.cpp src/robots_txt.cpp)

set(include_all ${include})
set(src_all ${src} src/str.cpp)

#lexbor - html parser   / https://github.com/lexbor/lexbor
#curl - http lib        / https://github.com/curl/curl
#OpenSSL - crypto lib   / https://github.com/openssl/openssl

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

if (NOT TARGET librengine)
    add_library(${PROJECT_NAME} SHARED ${include_all} ${src_all})
    target_link_libraries(${PROJECT_NAME} PUBLIC
            /usr/lib/liblexbor.so
            curl
            OpenSSL::Crypto
            Threads::Threads)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC include)

include(GNUInstallDirs)

install(FILES ${include} DESTINATION include/${PROJECT_NAME})
install(FILES ${src} DESTINATION src/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})