cmake_minimum_required(VERSION 3.10)

project(ui_sandbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IDEs / tooling (clangd, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW (optional, fallback to system OpenGL if not found)
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found, using system OpenGL")
endif()

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(ui_sandbox
    ${SRC_FILES}
)

# Link GLFW and OpenGL
if(glfw3_FOUND)
    target_link_libraries(ui_sandbox glfw)
    target_compile_definitions(ui_sandbox PRIVATE USE_GLFW)
endif()

# Always link OpenGL (required for OpenGL functions)
target_link_libraries(ui_sandbox 
    OpenGL::GL
)

# macOS specific frameworks (required for GLFW and OpenGL)
if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    target_link_libraries(ui_sandbox 
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
    )
endif()