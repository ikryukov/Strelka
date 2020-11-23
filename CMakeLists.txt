cmake_minimum_required(VERSION 3.16)
project(nevk)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
	# Application
	src/common.h
	src/main.cpp

	# Engine
	src/engine.h
	src/engine.cpp
	src/core/instance.h
	src/core/instance.cpp
	src/core/device.h
	src/core/device.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/)

##########################################
# External 3rd-party libraries

# GLFW (OpenGL Framework)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
add_subdirectory(external/glfw)

target_include_directories(${PROJECT_NAME} PRIVATE external/glfw/include)
target_link_directories(${PROJECT_NAME} PRIVATE external/glfw/src)
target_link_libraries(${PROJECT_NAME} glfw)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
