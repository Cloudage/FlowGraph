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

FetchContent_Declare(
        ExpressionKit
        GIT_REPOSITORY https://github.com/Cloudage/ExpressionKit
        GIT_TAG        v0.0.2
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(ExpressionKit)

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

# ExpressionKit is now available as ExpressionKit target from FetchContent_MakeAvailable

# Header-only library
add_library(FlowGraph INTERFACE)
add_library(FlowGraph::FlowGraph ALIAS FlowGraph)

# Include directories  
target_include_directories(FlowGraph INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/include>
    $<INSTALL_INTERFACE:include>
)

# Link to ExpressionKit target  
target_link_libraries(FlowGraph INTERFACE ExpressionKit)

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
        -DFlowGraph_VERSION=${PROJECT_VERSION}
        -DFLOWGRAPH_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DPACKAGE_OUTPUT_DIR=${CMAKE_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/package.cmake
    COMMENT "Creating FlowGraph distribution package"
    VERBATIM
)

# Installation
include(GNUInstallDirs)

# ExpressionKit will be handled by its own installation when using find_package

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