# Package generation script for FlowGraph
# This creates a self-contained header package that includes all dependencies

cmake_minimum_required(VERSION 3.15)

# Default package directory if not specified
if(NOT PACKAGE_OUTPUT_DIR)
    set(PACKAGE_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/package")
endif()

# Source directory passed from main CMakeLists.txt
if(NOT FLOWGRAPH_SOURCE_DIR)
    set(FLOWGRAPH_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
endif()

# Package version from project version
if(NOT PACKAGE_VERSION)
    set(PACKAGE_VERSION "${FlowGraph_VERSION}")
endif()

# Package name
set(PACKAGE_NAME "flowgraph-${PACKAGE_VERSION}")
set(PACKAGE_DIR "${PACKAGE_OUTPUT_DIR}/${PACKAGE_NAME}")

message(STATUS "Creating FlowGraph package in: ${PACKAGE_DIR}")

# Clean and create package directory
file(REMOVE_RECURSE "${PACKAGE_DIR}")
file(MAKE_DIRECTORY "${PACKAGE_DIR}")
file(MAKE_DIRECTORY "${PACKAGE_DIR}/include")
file(MAKE_DIRECTORY "${PACKAGE_DIR}/include/flowgraph")

# Copy FlowGraph headers
file(GLOB_RECURSE FLOWGRAPH_HEADERS 
    "${FLOWGRAPH_SOURCE_DIR}/Core/include/flowgraph/*.hpp"
)

foreach(HEADER ${FLOWGRAPH_HEADERS})
    file(RELATIVE_PATH REL_HEADER "${FLOWGRAPH_SOURCE_DIR}/Core/include" "${HEADER}")
    get_filename_component(HEADER_DIR "${PACKAGE_DIR}/include/${REL_HEADER}" DIRECTORY)
    file(MAKE_DIRECTORY "${HEADER_DIR}")
    file(COPY "${HEADER}" DESTINATION "${HEADER_DIR}")
endforeach()

# Copy ExpressionKit header
file(COPY "${EXPRESSIONKIT_SOURCE_DIR}/ExpressionKit.hpp" 
     DESTINATION "${PACKAGE_DIR}/include")

# Create package README
configure_file(
    "${FLOWGRAPH_SOURCE_DIR}/cmake/package_README.md.in"
    "${PACKAGE_DIR}/README.md"
    @ONLY
)

# Create a simple example
file(WRITE "${PACKAGE_DIR}/example.cpp" "
#include <iostream>
#include \"flowgraph/FlowGraph.hpp\"

int main() {
    std::cout << \"FlowGraph ${PACKAGE_VERSION} package example\" << std::endl;
    
    // Create a simple FlowGraph engine
    FlowGraph::FlowGraphEngine engine;
    
    std::cout << \"FlowGraph initialized successfully!\" << std::endl;
    return 0;
}
")

# Create package info file
file(WRITE "${PACKAGE_DIR}/PACKAGE_INFO.txt" "
FlowGraph Package Information
============================

Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Generated: ${CMAKE_CURRENT_LIST_FILE}

Contents:
- include/flowgraph/        FlowGraph headers
- include/ExpressionKit.hpp ExpressionKit dependency header
- example.cpp               Simple usage example
- README.md                 Package documentation

This package contains all necessary headers to use FlowGraph in your project.
No additional dependencies are required.
")

message(STATUS "Package created successfully: ${PACKAGE_DIR}")
message(STATUS "Package contents:")
file(GLOB_RECURSE PACKAGE_FILES "${PACKAGE_DIR}/*")
foreach(FILE ${PACKAGE_FILES})
    file(RELATIVE_PATH REL_FILE "${PACKAGE_DIR}" "${FILE}")
    message(STATUS "  ${REL_FILE}")
endforeach()