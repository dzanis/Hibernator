cmake_minimum_required(VERSION 2.8)

project(hibernator)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -static -std=c99 -mwindows  ") # -mwindows чтоб убрать окно консоли


add_executable(${PROJECT_NAME} ${SRC_FILES} )

target_link_libraries(${PROJECT_NAME} -lcomctl32)
