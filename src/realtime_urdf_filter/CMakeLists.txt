cmake_minimum_required (VERSION 3.10)

project(realtime_urdf_filter)

set(CMAKE_CXX_STANDARD 17) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

## Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif(WIN32)

link_directories(${CMAKE_SOURCE_DIR}/lib)

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

include_directories(includes)
set(LIBS ${GLFW3_LIBRARY}  ${ASSIMP_LIBRARY})
add_library(GLAD "src/glad.c")
set(LIBS ${LIBS} GLAD)

#find_library(OpenGL REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(tf2 REQUIRED)
find_package(urdf REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(image_transport REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

add_executable(urdf_filter
src/main.cpp
src/renderable.cpp
src/urdf_renderable.cpp
src/shader_wrapper.cpp
src/framebufferObject.cpp)

ament_target_dependencies(
urdf_filter
  "rclcpp"
  "tf2"
  "tf2_ros"
  "geometry_msgs"
  "urdf"
  "image_transport"
  "cv_bridge"
  "tf2_geometry_msgs"
)

target_link_libraries(
urdf_filter
  ${LIBS}
)

ament_package()

install(TARGETS urdf_filter
DESTINATION lib/${PROJECT_NAME})

install(
  DIRECTORY launch config urdf description
  DESTINATION share/${PROJECT_NAME}
)
