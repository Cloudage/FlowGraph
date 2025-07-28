cmake_minimum_required(VERSION 3.15)
project(FlowGraph VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add options
option(FLOWGRAPH_BUILD_TESTS "Build FlowGraph tests" ON)
option(FLOWGRAPH_BUILD_EXAMPLES "Build FlowGraph examples" ON)
option(BUILD_EDITOR "Build FlowGraph editor" OFF)
option(BUILD_EDITOR_TESTS "Build FlowGraph editor UI tests" OFF)

# Fetch ExpressionKit using FetchContent
include(FetchContent)

# Use legacy FetchContent_Populate to avoid building ExpressionKit CMake project
# which has compatibility issues. Suppress the deprecation warning.
cmake_policy(SET CMP0169 OLD)
FetchContent_Declare(
        ExpressionKit
        GIT_REPOSITORY https://github.com/Cloudage/ExpressionKit
        GIT_TAG        main
        GIT_SHALLOW    TRUE
)
FetchContent_Populate(ExpressionKit)
set(EXPRESSIONKIT_SOURCE_DIR ${expressionkit_SOURCE_DIR})

# Editor dependencies (only fetched when BUILD_EDITOR is enabled)
if(BUILD_EDITOR)
    # Dear ImGui
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.1
    )
    
    # GLFW for cross-platform windowing
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )
    
    # Platform-specific graphics dependencies
    if(UNIX AND NOT APPLE)
        # OpenGL loader for Linux
        FetchContent_Declare(
            glad
            GIT_REPOSITORY https://github.com/Dav1dde/glad.git
            GIT_TAG v0.1.36
        )
    endif()
    
    # ImGui Test Engine (for UI testing) - only when UI tests are enabled
    if(BUILD_EDITOR_TESTS)
        FetchContent_Declare(
            imgui_test_engine
            GIT_REPOSITORY https://github.com/ocornut/imgui_test_engine.git
            GIT_TAG main
        )
    endif()
    
    # Make dependencies available
    FetchContent_MakeAvailable(imgui glfw)
    if(BUILD_EDITOR_TESTS)
        FetchContent_MakeAvailable(imgui_test_engine)
    endif()
    if(UNIX AND NOT APPLE)
        FetchContent_MakeAvailable(glad)
    endif()
endif()

# Create ExpressionKit interface target
if(NOT TARGET ExpressionKit)
    add_library(ExpressionKit INTERFACE)
    target_include_directories(ExpressionKit INTERFACE 
        $<BUILD_INTERFACE:${EXPRESSIONKIT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_features(ExpressionKit INTERFACE cxx_std_17)
endif()

# Header-only library
add_library(FlowGraph INTERFACE)
add_library(FlowGraph::FlowGraph ALIAS FlowGraph)

# Include directories  
target_include_directories(FlowGraph INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/include>
    $<BUILD_INTERFACE:${EXPRESSIONKIT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Don't link to ExpressionKit target since we're bundling the header directly
# This maintains compatibility and avoids CMake target conflicts

# Compiler requirements
target_compile_features(FlowGraph INTERFACE cxx_std_17)

# Add tests if requested
if(FLOWGRAPH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Add examples if requested
if(FLOWGRAPH_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Add editor if requested
if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()

# Package generation target
add_custom_target(create_package
    COMMAND ${CMAKE_COMMAND} 
        -DEXPRESSIONKIT_SOURCE_DIR=${EXPRESSIONKIT_SOURCE_DIR}
        -DFlowGraph_VERSION=${PROJECT_VERSION}
        -DFLOWGRAPH_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DPACKAGE_OUTPUT_DIR=${CMAKE_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/package.cmake
    COMMENT "Creating FlowGraph distribution package"
    VERBATIM
)

# Installation
include(GNUInstallDirs)

# Install ExpressionKit header when we install FlowGraph
install(FILES ${EXPRESSIONKIT_SOURCE_DIR}/ExpressionKit.hpp 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS FlowGraph
    EXPORT FlowGraphTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY Core/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT FlowGraphTargets
    FILE FlowGraphTargets.cmake
    NAMESPACE FlowGraph::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FlowGraph
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    FlowGraphConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    cmake/FlowGraphConfig.cmake.in
    FlowGraphConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FlowGraph
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/FlowGraphConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FlowGraphConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FlowGraph
)