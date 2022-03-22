cmake_minimum_required(VERSION 3.20)
project(librengine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(include include/opensearch.h include/http.h include/str.h include/str_impl.h include/config.h include/logger.h include/json.hpp)
set(src src/opensearch.cpp src/http.cpp src/str.cpp src/logger.cpp src/config.cpp)

set(include_all ${include} ${tp_rep_cpp} ${tp_url_cpp})
set(src_all ${src})

#lexbor - html, css / https://github.com/lexbor/lexbor
#curl - http lib    / https://github.com/curl/curl

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
#[[target_link_libraries(${PROJECT_NAME} PUBLIC
        /usr/lib/liblexbor.so
        curl
        Threads::Threads)]]

add_library(${PROJECT_NAME} SHARED ${include_all} ${src_all})
target_include_directories(${PROJECT_NAME} PUBLIC include)

include(GNUInstallDirs)

install(FILES ${include} DESTINATION include/${PROJECT_NAME})
install(FILES ${src} DESTINATION src/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})