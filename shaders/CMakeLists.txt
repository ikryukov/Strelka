cmake_minimum_required(VERSION 3.16)
project(ShaderManager)

file(GLOB_RECURSE SHADERS *.slang)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Shaders" FILES ${SHADERS})

add_library(${PROJECT_NAME} STATIC
	ShaderManager.h
	ShaderManager.cpp
	${SHADERS}
)

# Slang
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/slang/include)
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/slang)
target_link_libraries(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/external/slang/slang.lib")

# Copy slang dll to final directory after building
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/external/slang/slang.dll"
        "${OUTPUT_DIRECTORY}"
)

# Copy shaders to final directory after building
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders/src"
        "${OUTPUT_DIRECTORY}/shaders"
)
