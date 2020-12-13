cmake_minimum_required(VERSION 3.16)
project(ShaderManager)

file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.slang)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Shaders" FILES ${SHADERS})

#add_library(${PROJECT_NAME} STATIC
#	${SHADERS}
#)

# Copy shaders to final directory after building
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders/src"
        "${OUTPUT_DIRECTORY}/shaders"
)
