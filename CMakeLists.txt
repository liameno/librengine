cmake_minimum_required(VERSION 3.2)
project(librengine)

add_subdirectory("${PROJECT_SOURCE_DIR}/cli" "${PROJECT_SOURCE_DIR}/cli/build")
add_subdirectory("${PROJECT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/lib/build")
add_subdirectory("${PROJECT_SOURCE_DIR}/crawler" "${PROJECT_SOURCE_DIR}/crawler/build")
add_subdirectory("${PROJECT_SOURCE_DIR}/website" "${PROJECT_SOURCE_DIR}/website/build")