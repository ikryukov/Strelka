cmake_minimum_required(VERSION 3.16)
project(Strelka)
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

if(WIN32)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()

######## Packages
# Vulkan
if (NOT DEFINED VULKAN_SDK)
    find_package(Vulkan REQUIRED)
    message(STATUS "VULKAN SDK package: ${Vulkan_LIBRARY}")
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
message(STATUS "Embree LIBRARY: ${EMBREE_LIBRARY}")

find_package(USD REQUIRED HINTS ${USD_DIR} NAMES pxr)
message(STATUS "USD LIBRARY: ${USD_DIR}")

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

find_package(MaterialX REQUIRED)

# Material Manager
set(MATERIALMANAGER_SOURCES
       include/materialmanager/materialmanager.h
       include/materialmanager/mdlNeurayLoader.h
       include/materialmanager/mdlRuntime.h
       include/materialmanager/mdlHlslCodeGen.h
       include/materialmanager/mtlxMdlCodeGen.h
       include/materialmanager/mdlLogger.h
       include/materialmanager/mdlMaterialCompiler.h

       src/materialmanager/materialmanager.cpp
       src/materialmanager/mdlNeurayLoader.cpp
       src/materialmanager/mdlMaterialCompiler.cpp
       src/materialmanager/mdlRuntime.cpp
       src/materialmanager/mdlHlslCodeGen.cpp
       src/materialmanager/mtlxMdlCodeGen.cpp
       src/materialmanager/mdlLogger.cpp
        )
set(MATERIALLIB_NAME materialmanager)
add_library(${MATERIALLIB_NAME} OBJECT ${MATERIALMANAGER_SOURCES})
target_include_directories(${MATERIALLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/ ${PROJECT_SOURCE_DIR}/include/materialmanager)
target_include_directories(${MATERIALLIB_NAME} PUBLIC  shaders/ external/glm)
target_link_libraries(${MATERIALLIB_NAME} PUBLIC ${SHADERLIB_NAME})
target_link_libraries(${MATERIALLIB_NAME} PRIVATE #hdMtlx )
        MaterialXCore
        MaterialXFormat
        MaterialXGenMdl)
# MDL ( $MDL_SDK_ROOT refer to the installation root of the MDL SDK.)
target_include_directories(${MATERIALLIB_NAME} PUBLIC external/mdl-sdk/include) # $ENV{MDL_SDK_ROOT}

target_include_directories(${MATERIALLIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${MATERIALLIB_NAME} PUBLIC ${Vulkan_LIBRARY})

# Scene
set(SCENE_SOURCES
        include/scene/scene.h
        include/scene/camera.h
        src/scene/scene.cpp
        src/scene/camera.cpp
        )
set(SCENELIB_NAME scene)
add_library(${SCENELIB_NAME} OBJECT ${SCENE_SOURCES})
target_include_directories(${SCENELIB_NAME} PUBLIC external/glm)
target_include_directories(${SCENELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${SCENELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/scene)
target_include_directories(${SCENELIB_NAME} PUBLIC shaders/)

# Settings
set(SETTINGSLIB_SOURCES
        include/settings/settings.h
        src/settings/settings.cpp
        )
set(SETTINGSLIB_NAME settings)
add_library(${SETTINGSLIB_NAME} OBJECT ${SETTINGSLIB_SOURCES})
target_include_directories(${SETTINGSLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${SETTINGSLIB_NAME} PUBLIC external/glm)

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
        include/render/bvh.h
        include/render/vkrender.h
        include/render/glfwrender.h
        include/render/ptrender.h
        include/render/shaderparameters.h
        include/render/computepass.h
        include/render/pathtracer.h
        include/render/upscalepass.h
        include/render/reduction.h
        include/render/tonemap.h
        include/render/accumulation.h
        include/render/debugview.h
        include/render/debugUtils.h
        src/render/vkrender.cpp
        src/render/glfwrender.cpp
        src/render/ptrender.cpp
        src/render/bvh.cpp
        src/render/upscalepass.cpp
        src/render/reduction.cpp
        src/render/pathtracer.cpp
        src/render/tonemap.cpp
        src/render/accumulation.cpp
        src/render/debugview.cpp
        src/render/debugUtils.cpp
        )
set(RENDERLIB_NAME render)
add_library(${RENDERLIB_NAME} OBJECT ${RENDER_SOURCES})
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/render)
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/shaders/)
target_include_directories(${RENDERLIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/external/tracy)
target_include_directories(${RENDERLIB_NAME} PUBLIC ${EMBREE_INCLUDE_PATH})

set(TEXTURELIB_SOURCES
        include/texturemanager/texturemanager.h
        src/texturemanager/texturemanager.cpp)
set(TEXTURELIB_NAME texturemanager)
add_library(${TEXTURELIB_NAME} OBJECT ${TEXTURELIB_SOURCES})
target_include_directories(${TEXTURELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${TEXTURELIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/texturemanager)
target_include_directories(${TEXTURELIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${TEXTURELIB_NAME} PUBLIC ${SCENELIB_NAME})

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

## Vulkan
target_include_directories(${RENDERLIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${Vulkan_LIBRARY})

# TOL (TinyObjLoader)
target_include_directories(${RENDERLIB_NAME} PUBLIC external/tol)

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
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${UILIB_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC ${TEXTURELIB_NAME})
target_link_libraries(${RENDERLIB_NAME} PUBLIC embree)
target_link_libraries(${RENDERLIB_NAME} PRIVATE 
    ${MATERIALLIB_NAME}
    MaterialXCore
    MaterialXFormat
    MaterialXGenMdl)

target_set_warnings(${RENDERLIB_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying)

# Application
set(HD_RUNNER_SOURCES
    src/hdRunner/main.cpp
    src/hdRunner/SimpleRenderTask.h
    src/hdRunner/SimpleRenderTask.cpp
)
add_executable(${PROJECT_NAME} ${HD_RUNNER_SOURCES})
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED TRUE
    CXX_EXTENSIONS OFF
)

# target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
# target_include_directories(${PROJECT_NAME} PUBLIC external/glfw/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${SETTINGSLIB_NAME}
    ${RESMNGR_NAME}
    ${SHADERLIB_NAME}
    ${MATERIALLIB_NAME}
    MaterialXCore
    MaterialXFormat
    MaterialXGenMdl
    ${SCENELIB_NAME}
    ${RENDERLIB_NAME}
    ${UILIB_NAME}
    ${TEXTURELIB_NAME}
    ${TRACY_LIB}
    ${Vulkan_LIBRARY}
    ar cameraUtil hd hf hgi hio usd usdGeom usdImaging    
)  # Link the executable to library (if it uses it).

# target_set_warnings(${PROJECT_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).

# Hydra plugin
set(HD_PLUGIN_SOURCES
    include/HdStrelka/RendererPlugin.h
    src/HdStrelka/RendererPlugin.cpp
    src/HdStrelka/RenderParam.h
    src/HdStrelka/RenderBuffer.h
    src/HdStrelka/RenderDelegate.h
    src/HdStrelka/RenderPass.h
    src/HdStrelka/RenderBuffer.cpp
    src/HdStrelka/RenderDelegate.cpp
    src/HdStrelka/RenderPass.cpp
    src/HdStrelka/Instancer.h
    src/HdStrelka/Instancer.cpp
    src/HdStrelka/Material.h
    src/HdStrelka/Material.cpp
    src/HdStrelka/Camera.h
    src/HdStrelka/Camera.cpp
    src/HdStrelka/Mesh.h
    src/HdStrelka/Mesh.cpp
    src/HdStrelka/Light.h
    src/HdStrelka/Light.cpp
    src/HdStrelka/Tokens.h
    src/HdStrelka/Tokens.cpp
    src/HdStrelka/MaterialNetworkTranslator.h
    src/HdStrelka/MaterialNetworkTranslator.cpp
    src/HdStrelka/MdlParserPlugin.h
    src/HdStrelka/MdlParserPlugin.cpp
    src/HdStrelka/MdlDiscoveryPlugin.h
    src/HdStrelka/MdlDiscoveryPlugin.cpp
)
set(HD_PLUGIN_NAME HdStrelka)
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

target_include_directories(${HD_PLUGIN_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/hdStrelka)
target_include_directories(${HD_PLUGIN_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src/hdStrelka)
target_include_directories(${HD_PLUGIN_NAME} PUBLIC hd)
target_include_directories(${HD_PLUGIN_NAME} PRIVATE 
    usdImaging
    hdMtlx
)

target_link_libraries(
  ${HD_PLUGIN_NAME}
  PRIVATE
    usdImaging
    hdMtlx
    ${RESMNGR_NAME} ${SHADERLIB_NAME} ${SCENELIB_NAME} ${RENDERLIB_NAME} ${UILIB_NAME} ${TEXTURELIB_NAME} ${TRACY_LIB}
    ${MATERIALLIB_NAME}
  PUBLIC
    hd
)

set(PLUGINFO_PATH "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json")

file(READ ${PROJECT_SOURCE_DIR}/src/hdStrelka/plugInfo.json.in PLUGINFO)
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
    "${CMAKE_INSTALL_PREFIX}/HdStrelka/resources"
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

# Set working directory
if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${OUTPUT_DIRECTORY}")
elseif (APPLE)
    set_property(TARGET ${PROJECT_NAME} PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${OUTPUT_DIRECTORY}")
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
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/external/mdl-sdk/nt-x86-64/lib/libmdl_sdk.dll"
            "${OUTPUT_DIRECTORY}"
            )
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/external/mdl-sdk/nt-x86-64/lib/nv_freeimage.dll"
            "${OUTPUT_DIRECTORY}"
            )
elseif(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/external/mdl-sdk/macosx-x86-64/lib/libmdl_sdk.so"
            "${OUTPUT_DIRECTORY}"
            )
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/external/mdl-sdk/macosx-x86-64/lib/nv_freeimage.so"
            "${OUTPUT_DIRECTORY}"
            )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/external/slang/libslang.dylib"
            "${OUTPUT_DIRECTORY}"
            )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/external/slang/libslang-glslang.dylib"
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

option(STRELKA_TESTS "enable tests" ON)

if (NOT STRELKA_TESTS)
    message(STATUS "tests are disabled")
else ()
    add_subdirectory(tests)
endif ()
