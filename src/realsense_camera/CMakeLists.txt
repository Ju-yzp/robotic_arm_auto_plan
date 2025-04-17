cmake_minimum_required(VERSION 3.10)

project(RealsenseCamera)

## Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME}
src/message_sync.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
include)

