cmake_minimum_required(VERSION 3.20)
project(librengine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(include include/opensearch.h include/http.h include/str.h include/str_impl.h)
set(include_crawler include/crawler/worker.h include/crawler/config.h)

set(src src/opensearch.cpp src/http.cpp src/str.cpp)
set(src_crawler src/crawler/worker.cpp)

set(tp_json third_party/json/json.hpp) #https://github.com/nlohmann/json
set(tp_rep_cpp third_party/rep-cpp/agent.cpp third_party/rep-cpp/agent.h
        third_party/rep-cpp/directive.cpp third_party/rep-cpp/directive.h third_party/rep-cpp/robots.cpp
        third_party/rep-cpp/robots.h) #robots.txt / https://github.com/seomoz/rep-cpp
set(tp_url_cpp third_party/url-cpp/psl.cpp third_party/url-cpp/psl.h
        third_party/url-cpp/punycode.cpp third_party/url-cpp/punycode.h third_party/url-cpp/url.cpp
        third_party/url-cpp/url.h third_party/url-cpp/utf8.cpp third_party/url-cpp/utf8.h) #https://github.com/seomoz/url-cpp

set(include_all ${include} ${include_crawler} ${tp_json} ${tp_rep_cpp} ${tp_url_cpp})
set(src_all ${src} ${src_crawler})

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
install(FILES ${tp_json} DESTINATION include/${PROJECT_NAME}/third_party/json)
install(FILES ${include_crawler} DESTINATION include/${PROJECT_NAME}/crawler)
install(FILES ${src} DESTINATION src/${PROJECT_NAME})
install(FILES ${src_crawler} DESTINATION src/${PROJECT_NAME}/crawler)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})