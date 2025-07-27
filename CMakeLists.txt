cmake_minimum_required(VERSION 3.15)
project(FlowGraph VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add options
option(FLOWGRAPH_BUILD_TESTS "Build FlowGraph tests" ON)
option(FLOWGRAPH_BUILD_EXAMPLES "Build FlowGraph examples" ON)

# Fetch ExpressionKit using FetchContent
include(FetchContent)
FetchContent_Declare(
    ExpressionKit
    # Note: This would normally point to the actual ExpressionKit repository
    # For now, we'll create a minimal header-only target from the local file
    # URL https://github.com/example/ExpressionKit.git
    # GIT_TAG main
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ExpressionKit
)

# For this transition, we'll create a minimal target that points to our current header
if(NOT TARGET ExpressionKit::ExpressionKit)
    add_library(ExpressionKit INTERFACE)
    add_library(ExpressionKit::ExpressionKit ALIAS ExpressionKit)
    target_include_directories(ExpressionKit INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Header-only library
add_library(FlowGraph INTERFACE)
add_library(FlowGraph::FlowGraph ALIAS FlowGraph)

# Include directories
target_include_directories(FlowGraph INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/include>
    $<INSTALL_INTERFACE:include>
)

# Link ExpressionKit
target_link_libraries(FlowGraph INTERFACE ExpressionKit::ExpressionKit)

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

# Installation
include(GNUInstallDirs)
install(TARGETS FlowGraph ExpressionKit
    EXPORT FlowGraphTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY Core/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES external/ExpressionKit.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

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