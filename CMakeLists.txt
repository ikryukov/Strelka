cmake_minimum_required(VERSION 3.16)
project(myopengl)

link_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(NAME "app")
add_executable(${NAME} src/main.cpp)

if (WIN32)
    target_link_libraries(${NAME} opengl32 glfw3 glew32s)

elseif(UNIX AND NOT APPLE)

    set(_find_paths_header_ 
        "/usr/include"
        "/usr/local/include"
        "${CMAKE_SOURCE_DIR}/include")
    set(_find_paths_lib_
        "/usr/lib"
        "/usr/local/lib"
        "${CMAKE_SOURCE_DIR}/lib")

    # Find GLFW3
    find_path(GLFW3_INCLUDE_DIR "GLFW/glfw3.h" PATHS ${_find_paths_header_})
    find_library(GLFW3_LIBRARIES NAMES glfw3 glfw PATHS ${_find_paths_lib_})
    message(STATUS "Found GLFW3: ${GLFW3_INCLUDE_DIR}")

    # Find GLEW
    find_package(GLEW REQUIRED)

    target_link_libraries(${NAME} ${GLFW3_LIBRARIES} ${GLEW_LIBRARIES} X11 GL pthread)
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

elseif(APPLE)
    # Link system libs
    INCLUDE_DIRECTORIES(/System/Library/Frameworks)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
    FIND_LIBRARY(IOKit_LIBRARY IOKit)
    FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
    MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
    SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
    SET(APPLE_LIBS ${APPLE_LIBS} ${ASSIMP_LIBRARY})
    set(LIBS ${LIBS} ${APPLE_LIBS})

    find_package(OpenGL REQUIRED)
    # Find GLEW
    find_package(GLEW REQUIRED)

    # Find GLFW3
    set(_find_paths_header_
            "/usr/local/include"
            "${CMAKE_SOURCE_DIR}/include")

    # Find GLFW3
    find_path(GLFW3_INCLUDE_DIR "GLFW/glfw3.h" PATHS ${_find_paths_header_})
    message(STATUS "Found GLFW3: ${GLFW3_INCLUDE_DIR}")


    target_link_libraries(${NAME} ${GLFW3_LIBRARIES} GLEW::GLEW)

endif(WIN32)
