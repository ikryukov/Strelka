cmake_minimum_required(VERSION 3.18)
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
    # TODO
endif(WIN32)
