cmake_minimum_required(VERSION 3.10)
project(crawler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp src/worker.cpp include/worker.h
        src/json_generator.cpp include/json_generator.h src/html_helper.cpp
        include/html_helper.h ${tp_rep_cpp} ${tp_url_cpp})

if (NOT TARGET librengine)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE librengine)