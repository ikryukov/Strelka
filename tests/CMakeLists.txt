cmake_minimum_required(VERSION 3.8.2)

set(TESTFILES
        tests_main.cpp
        render/tests_render.cpp
        scene/tests_scene.cpp
        shadermanager/tests_shadermanager.cpp
        modelloader/test_model.cpp
        ui/tests_ui.cpp
        intersection/aabb.cpp
        materialmanager/test_materialmanager.cpp
        )

set(TEST_MAIN unit_tests)
set(TEST_RUNNER_PARAMS "")

add_executable(${TEST_MAIN} ${TESTFILES})
target_link_libraries(${TEST_MAIN} PRIVATE ${RESMNGR_NAME} ${SHADERLIB_NAME} ${SCENELIB_NAME} 
        ${RENDERLIB_NAME} ${UILIB_NAME} ${MODELLIB_NAME} ${TEXTURELIB_NAME} ${TRACY_LIB} ${MATERIALLIB_NAME} doctest)
target_include_directories(${TEST_MAIN} PRIVATE ../include/)

# Set working directory
set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if (MSVC)
    set_property(TARGET ${TEST_MAIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${OUTPUT_DIRECTORY}")
elseif (APPLE)
    set_property(TARGET ${TEST_MAIN} PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${OUTPUT_DIRECTORY}")
endif ()
set_target_properties(${TEST_MAIN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
# target_set_warnings(${TEST_MAIN} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).

set_target_properties(${TEST_MAIN} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

add_test(
        NAME ${RENDERLIB_NAME}.${TEST_MAIN}
        COMMAND ${TEST_MAIN} ${TEST_RUNNER_PARAMS})
