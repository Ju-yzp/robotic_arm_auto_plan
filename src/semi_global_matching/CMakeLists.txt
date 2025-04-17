cmake_minimum_required(VERSION 3.10)

project(SemiGlobalMatching)

set(CMAKE_CXX_STANDARD 17)

# 检查编译器类型
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # GCC或Clang编译器，添加 -mavx2 选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC编译器，添加 /arch:AVX2 选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
endif()


## Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 启用 AddressSanitizer
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    
    # 添加调试符号（方便定位问题）
    add_compile_options(-g)
    
    # 关闭优化以提升检测准确性
    add_compile_options(-O0)
endif()

find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME}
test/sgm_test.cpp
src/SemiGlobalMatching.cpp
src/sgm_util.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
${OpenCV_INCLUDE_DIR}
include)

target_link_libraries(${PROJECT_NAME} PRIVATE
${OpenCV_LIBS})

INSTALL(TARGETS ${PROJECT_NAME}
DESTINATION lib)