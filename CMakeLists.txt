cmake_minimum_required(VERSION 3.18)
project(myopengl)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)

# External 3rd-party libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_SOURCE_DIR}/external/glfw/include
)

target_link_directories(${CMAKE_PROJECT_NAME}
    PRIVATE external/glfw/src
)

if(WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} opengl32 glfw)
elseif(UNIX AND NOT APPLE)
    #TODO
elseif(APPLE)
    #TODO
endif(WIN32)
