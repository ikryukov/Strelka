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

if (MINGW)
	set(IMGUI_FILES
			external/imgui/src/imgui.cpp
			external/imgui/src/imgui_demo.cpp
			external/imgui/src/imgui_draw.cpp
			external/imgui/src/imgui_widgets.cpp
			external/imgui/src/imgui_impl_glfw.cpp
			external/imgui/src/imgui_impl_vulkan.cpp
			)
endif()

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

	#imgui
		${IMGUI_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/)

##########################################
# External 3rd-party libraries

# GLFW (OpenGL Framework)
target_include_directories(${PROJECT_NAME} PRIVATE external/glfw/include)
if (MSVC)
	target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/glfw3.lib")
elseif (MINGW)
	target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/libglfw3.a")
elseif (APPLE)
	find_package(glfw3 3.3 REQUIRED)
    include_directories(${glfw3_INCLUDE_DIRS})
    target_link_libraries (${PROJECT_NAME} glfw)
endif()

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

# Slang (Shader manager)
add_subdirectory(shaders)
target_link_libraries(${PROJECT_NAME} ShaderManager)

# ImGUI
target_include_directories(${PROJECT_NAME} PRIVATE external/imgui/include)
if (MSVC)
	target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.lib")
endif()

# VMA (Vulkan Memory Allocator)
target_include_directories(${PROJECT_NAME} PRIVATE external/vma/src)

# GLM (OpenGL Mathematics)
target_include_directories(${PROJECT_NAME} PRIVATE external/glm)

# STB (image loader)
target_include_directories(${PROJECT_NAME} PRIVATE external/stb)

# TOL (TinyObjLoader)
target_include_directories(${PROJECT_NAME} PRIVATE external/tol)

# SPDLOG
target_include_directories(${PROJECT_NAME} PRIVATE external/spdlog/include)

# CXXOPTS
target_include_directories(${PROJECT_NAME} PRIVATE external/cxxopts/include)

