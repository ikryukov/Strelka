cmake_minimum_required(VERSION 3.16)
project(nevk)
set(CMAKE_CXX_STANDARD 17)

set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set output directory for executable files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})

# Set output directory for static libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})

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
target_include_directories(${PROJECT_NAME} PRIVATE external/glfw/include)
target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/glfw3.lib")

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

# Slang (Shader manager)
add_subdirectory(shaders)
target_link_libraries(${PROJECT_NAME} ShaderManager)

# ImGUI
target_include_directories(${PROJECT_NAME} PRIVATE external/imgui/include)
target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.lib")

# VMA (Vulkan Memory Allocator)
target_include_directories(${PROJECT_NAME} PRIVATE external/vma/src)

# GLM (OpenGL Mathematics)
target_include_directories(${PROJECT_NAME} PRIVATE external/glm)

# STB (image loader)
target_include_directories(${PROJECT_NAME} PRIVATE external/stb)

# TOL (TinyObjLoader)
target_include_directories(${PROJECT_NAME} PRIVATE external/tol)
