cmake_minimum_required(VERSION 3.16)
project(nevk)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(CTest)
include(Doctest)
include(Warnings)

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

######## Packages
# Vulkan
find_package(Vulkan REQUIRED)

# Shader Manager
set(SHADERMANAGER_SOURCES          # All .cpp files in src/
    src/shadermanager/shadermanager.cpp
)
set(SHADERLIB_NAME shadermanager)
add_library(${SHADERLIB_NAME} OBJECT ${SHADERMANAGER_SOURCES})
target_include_directories(${SHADERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${SHADERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/shadermanager)
target_include_directories(${SHADERLIB_NAME} PUBLIC external/slang/include)
if (MSVC)
	target_link_libraries(${SHADERLIB_NAME} "${CMAKE_SOURCE_DIR}/external/slang/slang.lib")
elseif (APPLE)
	target_link_libraries(${SHADERLIB_NAME} "${CMAKE_SOURCE_DIR}/external/slang/libslang.dylib")
endif()

# Scene
set(SCENE_SOURCES          # All .cpp files in src/
    src/scene/scene.cpp
)
set(SCENELIB_NAME scene)
add_library(${SCENELIB_NAME} OBJECT ${SCENE_SOURCES})
target_include_directories(${SCENELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${SCENELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/scene)
target_include_directories(${SCENELIB_NAME} PRIVATE external/glm)

# Render lib
set(RENDER_SOURCES          # All .cpp files in src/
    src/render/render.cpp
)
set(RENDERLIB_NAME render)
add_library(${RENDERLIB_NAME} OBJECT ${RENDER_SOURCES})
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
# GLFW
target_include_directories(${RENDERLIB_NAME} PRIVATE external/glfw/include)
if (MSVC)
	target_link_libraries(${RENDERLIB_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/glfw3.lib")
elseif (MINGW)
	target_link_libraries(${RENDERLIB_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/libglfw3.a")
elseif (APPLE)
	find_package(glfw3 3.3 REQUIRED)
    include_directories(${glfw3_INCLUDE_DIRS})
    target_link_libraries(${RENDERLIB_NAME} PUBLIC glfw)
endif()
#target_include_directories(${RENDERLIB_NAME} PUBLIC Vulkan::Vulkan)
target_link_libraries(${RENDERLIB_NAME} PUBLIC Vulkan::Vulkan)
target_link_libraries(${RENDERLIB_NAME} PUBLIC doctest)
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${SHADERLIB_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${SCENELIB_NAME})
#target_set_warnings(${RENDERLIB_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying)

# Application
add_executable(${PROJECT_NAME} app/main.cpp)
target_link_libraries(${PROJECT_NAME}  ${RENDERLIB_NAME})  # Link the executable to library (if it uses it).
#target_set_warnings(${PROJECT_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).

##########################################
# External 3rd-party libraries

# Slang (Shader manager)
# add_subdirectory(shaders)
# target_link_libraries(${PROJECT_NAME} ShaderManager)

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

option(NEVK_TESTS "enable tests" ON)

if(NOT NEVK_TESTS)
    message(STATUS "tests are disabled")
else()
    add_subdirectory(tests)
endif()