cmake_minimum_required(VERSION 3.20)
project(cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)

if (NOT TARGET librengine)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE librengine)