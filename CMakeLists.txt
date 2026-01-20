cmake_minimum_required(VERSION 3.10)

project(ui_sandbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IDEs / tooling (clangd, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(ui_sandbox
    ${SRC_FILES}
)

# Tests
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    
    # Download Catch2 single header if not present
    set(CATCH2_VERSION "2.13.10")
    set(CATCH2_URL "https://raw.githubusercontent.com/catchorg/Catch2/v${CATCH2_VERSION}/single_include/catch2/catch.hpp")
    set(CATCH2_PATH "${CMAKE_SOURCE_DIR}/test/catch.hpp")
    
    if(NOT EXISTS ${CATCH2_PATH})
        message(STATUS "Catch2 not found, downloading...")
        file(DOWNLOAD ${CATCH2_URL} ${CATCH2_PATH} SHOW_PROGRESS STATUS DOWNLOAD_STATUS)
        if(NOT DOWNLOAD_STATUS EQUAL "0")
            message(WARNING "Failed to download Catch2. Tests may not compile.")
        endif()
    endif()
    
    # Test files (exclude files that define CATCH_CONFIG_MAIN)
    file(GLOB TEST_FILES
        "${CMAKE_SOURCE_DIR}/test/*Test.cpp"
    )
    
    # Create test executable for each test file
    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        
        add_executable(${TEST_NAME}
            ${TEST_FILE}
        )
        
        # Only link source files that don't have main()
        # Exclude main.cpp from test builds
        file(GLOB_RECURSE TEST_SRC_FILES
            "${CMAKE_SOURCE_DIR}/src/*.cpp"
        )
        list(REMOVE_ITEM TEST_SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")
        
        target_sources(${TEST_NAME} PRIVATE ${TEST_SRC_FILES})
        
        target_include_directories(${TEST_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/test
        )
        
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
endif()