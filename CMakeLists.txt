cmake_minimum_required(VERSION 3.16)
project(nevk)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
add_compile_definitions(TRACY_ENABLE)
include(ConfigSafeGuards)
include(CTest)
include(Doctest)
include(Warnings)

option(ENABLE_WARNINGS_SETTINGS "Allow target_set_warnings to add flags and defines.
                                 Set this to OFF if you want to provide your own warning parameters." ON)

set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set output directory for executable files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})

if (MINGW)
    set(IMGUI_FILES
            external/imgui/src/imgui.cpp
            external/imgui/src/imgui_demo.cpp
            external/imgui/src/imgui_draw.cpp
            external/imgui/src/imgui_widgets.cpp
            external/imgui/src/imgui_impl_glfw.cpp
            external/imgui/src/imgui_impl_vulkan.cpp
            )
endif ()

######## Packages
# Vulkan
if (NOT DEFINED VULKAN_SDK)
    find_package(Vulkan REQUIRED)
else ()
# Workaround for CI
if (APPLE)
    set(Vulkan_LIBRARY ${VULKAN_SDK}/lib/libvulkan.dylib)
elseif (MSVC)
    set(Vulkan_LIBRARY ${VULKAN_SDK}/lib/vulkan-1.lib)
endif ()
    set(Vulkan_INCLUDE_DIR ${VULKAN_SDK}/include)
    message(STATUS "VULKAN LIBRARY: ${Vulkan_LIBRARY}")
    message(STATUS "VULKAN INCLUDE: ${Vulkan_INCLUDE_DIR}")
endif ()

# Embree
find_package(embree 3 REQUIRED)
message(STATUS "VULKAN LIBRARY: ${Vulkan_LIBRARY}")

find_package(USD REQUIRED HINTS ${USD_ROOT} NAMES pxr)

# Shader Manager
set(SHADERMANAGER_SOURCES
        include/shadermanager/shadermanager.h
        src/shadermanager/shadermanager.cpp
        )
set(SHADERLIB_NAME shadermanager)
add_library(${SHADERLIB_NAME} OBJECT ${SHADERMANAGER_SOURCES})
target_include_directories(${SHADERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${SHADERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/shadermanager shaders/)
target_include_directories(${SHADERLIB_NAME} PUBLIC external/slang/include)
if (MSVC)
    target_link_libraries(${SHADERLIB_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/external/slang/slang.lib")
elseif (APPLE)
    target_link_libraries(${SHADERLIB_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/external/slang/libslang.dylib")
    target_link_libraries(${SHADERLIB_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/external/slang/libslang-glslang.dylib")
endif ()

# Scene
set(SCENE_SOURCES
        include/scene/scene.h
        include/scene/camera.h
        src/scene/scene.cpp
        src/scene/camera.cpp
        )
set(SCENELIB_NAME scene)
add_library(${SCENELIB_NAME} OBJECT ${SCENE_SOURCES})
target_include_directories(${SCENELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${SCENELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/scene)
target_include_directories(${SCENELIB_NAME} PUBLIC shaders/)
target_include_directories(${SCENELIB_NAME} PUBLIC external/glm)

# UI
set(UI_SOURCES
        include/ui/ui.h
        src/ui/ui.cpp
        # Imgui
        external/imgui/include
        external/imgui/src/imgui.cpp
        external/imgui/src/imgui_demo.cpp
        external/imgui/src/imgui_draw.cpp
        external/imgui/src/imgui_widgets.cpp
        external/imgui/src/imgui_tables.cpp
        external/imgui/src/imgui_impl_vulkan.cpp
        external/imgui/src/imgui_impl_glfw.cpp
        # imguiFD
        external/imguiFD/ImGuiFileDialog.cpp
        # ImGuizmo
        external/ImGuizmo/ImGuizmo.cpp
        )
set(UILIB_NAME ui)
add_library(${UILIB_NAME} OBJECT ${UI_SOURCES})
target_include_directories(${UILIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${UILIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/ui)
target_include_directories(${UILIB_NAME} PUBLIC shaders/)
target_include_directories(${UILIB_NAME} PUBLIC external/imgui/include)
target_include_directories(${UILIB_NAME} PRIVATE external/glfw/include)
target_include_directories(${UILIB_NAME} PUBLIC external/ImGuizmo)
target_include_directories(${UILIB_NAME} PUBLIC external/glm)
if (MSVC)
    target_link_libraries(${UILIB_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/glfw3.lib")
endif ()
## Vulkan
target_include_directories(${UILIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${UILIB_NAME} PUBLIC ${Vulkan_LIBRARY})
# JSON
target_include_directories(${UILIB_NAME} PUBLIC external/json)

# Resource Manager lib
set(RESMNGR_SOURCES
        include/resourcemanager/resourcemanager.h
        src/resourcemanager/resourcemanager.cpp
        )
set(RESMNGR_NAME resourcemanager)
add_library(${RESMNGR_NAME} OBJECT ${RESMNGR_SOURCES})
target_include_directories(${RESMNGR_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${RESMNGR_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/resourcemanager)
## VMA
target_include_directories(${RESMNGR_NAME} PUBLIC external/vma/src)
## Vulkan
target_include_directories(${RESMNGR_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${RESMNGR_NAME} PUBLIC ${Vulkan_LIBRARY})

# Tracy profiler
set(TRACY_SOURCES
        external/tracy/TracyClient.cpp
        external/tracy/Tracy.hpp)
set(TRACY_LIB tracy)
add_library(${TRACY_LIB} OBJECT ${TRACY_SOURCES})

# Render lib
set(RENDER_SOURCES
        include/render/common.h
        include/render/gbuffer.h
        include/render/bvh.h
        include/render/render.h
        include/render/ptrender.h
        include/render/renderpass.h
        include/render/gbufferpass.h
        include/render/shaderparameters.h
        include/render/computepass.h
        include/render/rtshadowpass.h
        include/render/pathtracer.h
        include/render/reflection.h
        include/render/aopass.h
        include/render/upscalepass.h
        include/render/ltcpass.h
        include/render/bilateralfilter.h
        include/render/tonemap.h
        include/render/composition.h
        include/render/accumulation.h
        include/render/debugview.h
        include/render/depthpass.h
        include/render/debugUtils.h
        src/render/render.cpp
        src/render/ptrender.cpp
        src/render/renderpass.cpp
        src/render/gbufferpass.cpp
        src/render/bvh.cpp
        src/render/upscalepass.cpp
        src/render/rtshadowpass.cpp
        src/render/pathtracer.cpp
        src/render/reflection.cpp
        src/render/aopass.cpp
        src/render/ltcpass.cpp
        src/render/bilateralfilter.cpp
        src/render/ltc_data.h
        src/render/tonemap.cpp
        src/render/composition.cpp
        src/render/accumulation.cpp
        src/render/debugview.cpp
        src/render/depthpass.cpp
        src/render/debugUtils.cpp
        )
set(RENDERLIB_NAME render)
add_library(${RENDERLIB_NAME} OBJECT ${RENDER_SOURCES})
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/render)
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/shaders/)
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/external/tracy)
target_include_directories(${RENDERLIB_NAME} PUBLIC ${EMBREE_INCLUDE_PATH})

# Model loader lib
set(MODELLIB_SOURCES
        include/modelloader/modelloader.h
        src/modelloader/modelloader.cpp)
set(MODELLIB_NAME modelloader)
add_library(${MODELLIB_NAME} OBJECT ${MODELLIB_SOURCES})
target_include_directories(${MODELLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${MODELLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/modelloader)
target_link_libraries(${MODELLIB_NAME} PUBLIC ${SCENELIB_NAME})
target_include_directories(${MODELLIB_NAME} PUBLIC external/tinygltf)

set(TEXTURELIB_SOURCES
        include/texturemanager/texturemanager.h
        src/texturemanager/texturemanager.cpp)
set(TEXTURELIB_NAME texturemanager)
add_library(${TEXTURELIB_NAME} OBJECT ${TEXTURELIB_SOURCES})
target_include_directories(${TEXTURELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${TEXTURELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/texturemanager)
target_link_libraries(${TEXTURELIB_NAME} PUBLIC ${SCENELIB_NAME} ${MODELLIB_NAME})

## Vulkan
target_include_directories(${RENDERLIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${Vulkan_LIBRARY})
target_include_directories(${MODELLIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${MODELLIB_NAME} PUBLIC ${Vulkan_LIBRARY})

## GLFW
target_include_directories(${RENDERLIB_NAME} PUBLIC external/glfw/include)
if (MSVC)
    target_link_libraries(${RENDERLIB_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/glfw3.lib")
elseif (MINGW)
    target_link_libraries(${RENDERLIB_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/libglfw3.a")
elseif (APPLE)
    find_package(glfw3 3.3 REQUIRED)
    include_directories(${glfw3_INCLUDE_DIRS})
    target_link_libraries(${RENDERLIB_NAME} PUBLIC glfw)
endif ()

## GLM (Mathematics)
target_include_directories(${RENDERLIB_NAME} PUBLIC external/glm)
target_include_directories(${MODELLIB_NAME} PUBLIC external/glm)

# TOL (TinyObjLoader)
target_include_directories(${RENDERLIB_NAME} PUBLIC external/tol)
target_include_directories(${MODELLIB_NAME} PUBLIC external/tol)

# STB (image loader)
target_include_directories(${RENDERLIB_NAME} PUBLIC external/stb)
target_include_directories(${TEXTURELIB_NAME} PUBLIC external/stb)

# imguiFD
target_include_directories(${RENDERLIB_NAME} PUBLIC external/imguiFD)
target_include_directories(${UILIB_NAME} PUBLIC external/imguiFD)

target_link_libraries(${RENDERLIB_NAME} PUBLIC doctest)
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${RESMNGR_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${SHADERLIB_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${SCENELIB_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${MODELLIB_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${UILIB_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${TEXTURELIB_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC embree)
target_set_warnings(${RENDERLIB_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying)

# Application
add_executable(${PROJECT_NAME} app/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${RESMNGR_NAME} ${SHADERLIB_NAME} ${SCENELIB_NAME} ${RENDERLIB_NAME} ${UILIB_NAME} ${MODELLIB_NAME} ${TEXTURELIB_NAME} ${TRACY_LIB})  # Link the executable to library (if it uses it).
target_set_warnings(${PROJECT_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).

# Hydra plugin
set(HD_PLUGIN_SOURCES
    include/HdNeVK/RendererPlugin.h
    src/HdNeVK/RendererPlugin.cpp
    src/HdNeVK/RenderBuffer.h
    src/HdNeVK/RenderDelegate.h
    src/HdNeVK/RenderPass.h
    src/HdNeVK/RenderBuffer.cpp
    src/HdNeVK/RenderDelegate.cpp
    src/HdNeVK/RenderPass.cpp
    src/HdNeVK/Instancer.h
    src/HdNeVK/Instancer.cpp
    src/HdNeVK/Material.h
    src/HdNeVK/Material.cpp
    src/HdNeVK/Camera.h
    src/HdNeVK/Camera.cpp
    src/HdNeVK/Mesh.h
    src/HdNeVK/Mesh.cpp
    src/HdNeVK/Tokens.h
    src/HdNeVK/Tokens.cpp
    src/HdNeVK/MaterialNetworkTranslator.h
    src/HdNeVK/MaterialNetworkTranslator.cpp    
)
set(HD_PLUGIN_NAME HdNeVK)
add_library(${HD_PLUGIN_NAME} SHARED ${HD_PLUGIN_SOURCES})
set_target_properties(
  ${HD_PLUGIN_NAME}
  PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    INSTALL_RPATH_USE_LINK_PATH ON
    # The other libs in the plugin dir have no "lib" prefix, so let's match this
    PREFIX ""
)

if(WIN32)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(_USE_MATH_DEFINES)
  add_compile_definitions(_ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH)
endif()

target_compile_definitions(
  ${HD_PLUGIN_NAME}
  PUBLIC
    # Required for PLUG_THIS_PLUGIN macro
    MFB_PACKAGE_NAME=${HD_PLUGIN_NAME}
  PRIVATE
    # Workaround for https://github.com/PixarAnimationStudios/USD/issues/1471#issuecomment-799813477
    "$<$<OR:$<CONFIG:>,$<CONFIG:Debug>>:TBB_USE_DEBUG>"
)

# Workaround for https://github.com/PixarAnimationStudios/USD/issues/1279
if (MSVC_VERSION GREATER_EQUAL 1920)
    target_compile_options(${HD_PLUGIN_NAME} PRIVATE "/Zc:inline-")
endif()
set(Boost_USE_STATIC_LIBS OFF) 
set(BUILD_SHARED_LIBS ON)
# find_package(Boost COMPONENTS python REQUIRED)

target_include_directories(${HD_PLUGIN_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/hdNeVK)
target_include_directories(${HD_PLUGIN_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src/hdNeVK)
target_include_directories(${HD_PLUGIN_NAME} PUBLIC hd)
target_include_directories(${HD_PLUGIN_NAME} PRIVATE 
    usdImaging
    hdMtlx
)

target_link_libraries(
  ${HD_PLUGIN_NAME}
  PRIVATE
    # ${Boost_LIBRARIES}
    usdImaging
    hdMtlx
    ${RESMNGR_NAME} ${SHADERLIB_NAME} ${SCENELIB_NAME} ${RENDERLIB_NAME} ${UILIB_NAME} ${MODELLIB_NAME} ${TEXTURELIB_NAME} ${TRACY_LIB}
  PUBLIC
    hd
)

set(PLUGINFO_PATH "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json")

file(READ ${PROJECT_SOURCE_DIR}/src/hdNeVK/plugInfo.json.in PLUGINFO)
file(
  GENERATE
  OUTPUT
    "${PLUGINFO_PATH}"
  CONTENT
    ${PLUGINFO}
)

install(
  FILES
    "${PLUGINFO_PATH}"
  DESTINATION
    "${CMAKE_INSTALL_PREFIX}/HdNeVK/resources"
  COMPONENT
    ${HD_PLUGIN_NAME}
)

install(
  TARGETS
    ${HD_PLUGIN_NAME}
  LIBRARY
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
    COMPONENT ${HD_PLUGIN_NAME}
  RUNTIME
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
    COMPONENT ${HD_PLUGIN_NAME}
  ARCHIVE
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
    COMPONENT ${HD_PLUGIN_NAME}
)

# Hydra runner application
set(HD_RUNNER_SOURCES
    src/hdRunner/main.cpp
    src/hdRunner/SimpleRenderTask.h
    src/hdRunner/SimpleRenderTask.cpp
)
set(HD_RUNNER_NAME HdRunner)
add_executable(${HD_RUNNER_NAME} ${HD_RUNNER_SOURCES})
set_target_properties(
  ${HD_RUNNER_NAME}
  PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED TRUE
    CXX_EXTENSIONS OFF
)

target_link_libraries(
  ${HD_RUNNER_NAME}
  ar cameraUtil hd hf hgi hio usd usdGeom usdImaging
)

# Set working directory
if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${OUTPUT_DIRECTORY}")
    set_property(TARGET ${HD_RUNNER_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${OUTPUT_DIRECTORY}")
elseif (APPLE)
    set_property(TARGET ${PROJECT_NAME} PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${OUTPUT_DIRECTORY}")
    set_property(TARGET ${HD_RUNNER_NAME} PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${OUTPUT_DIRECTORY}")
endif ()

# Create symbolic links in final directory before building
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_SOURCE_DIR}/shaders"
        "${OUTPUT_DIRECTORY}/shaders"
        )
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_SOURCE_DIR}/misc"
        "${OUTPUT_DIRECTORY}/misc"
        )

# Copy slang dlls to final directory after building
if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/external/slang/slang.dll"
            "${OUTPUT_DIRECTORY}"
            )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/external/slang/slang-glslang.dll"
            "${OUTPUT_DIRECTORY}"
            )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${EMBREE_ROOT_DIR}/bin/tbb12.dll"
            "${OUTPUT_DIRECTORY}"
            )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${EMBREE_ROOT_DIR}/bin/embree3.dll"
            "${OUTPUT_DIRECTORY}"
            )
endif ()

##########################################
# External 3rd-party libraries

# Slang (Shader manager)
# add_subdirectory(shaders)
# target_link_libraries(${PROJECT_NAME} ShaderManager)

# ImGUI
# target_include_directories(${PROJECT_NAME} PRIVATE external/imgui/include)
# if (MSVC)
#	target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.lib")
# endif()

# VMA (Vulkan Memory Allocator)
# target_include_directories(${PROJECT_NAME} PRIVATE external/vma/src)

# GLM (OpenGL Mathematics)
# target_include_directories(${PROJECT_NAME} PRIVATE external/glm)


# TOL (TinyObjLoader)
# target_include_directories(${PROJECT_NAME} PRIVATE external/tol)

# SPDLOG
# target_include_directories(${PROJECT_NAME} PRIVATE external/spdlog/include)

# CXXOPTS
target_include_directories(${PROJECT_NAME} PRIVATE external/cxxopts/include)

option(NEVK_TESTS "enable tests" ON)

if (NOT NEVK_TESTS)
    message(STATUS "tests are disabled")
else ()
    add_subdirectory(tests)
endif ()
