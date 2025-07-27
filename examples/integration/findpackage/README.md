# FlowGraph find_package Integration Example

This example shows how to use FlowGraph via CMake find_package after installation.

## Prerequisites

FlowGraph must be installed system-wide first:

```bash
# From the FlowGraph root directory
cmake -B build
cmake --build build
sudo cmake --install build
```

## Usage

Once FlowGraph is installed:

```bash
cmake -B build
cmake --build build
./build/findpackage_example
```

## What This Example Demonstrates

- Using FlowGraph via CMake find_package
- Working with installed FlowGraph libraries
- Complete FlowGraph functionality including:
  - Engine creation
  - ExpressionKit value types
  - Expression evaluation with variables
  - AST creation and node connections

## Integration Method

This approach is ideal when:
- You want to install FlowGraph system-wide
- Multiple projects need to use FlowGraph
- You prefer traditional package management
- You're distributing software that depends on FlowGraph

## CMakeLists.txt Explanation

The CMakeLists.txt is very simple:

```cmake
find_package(FlowGraph REQUIRED)
target_link_libraries(your_target PRIVATE FlowGraph::FlowGraph)
```

## Benefits

- Clean separation between FlowGraph and your project
- Standard CMake package discovery
- Works with package managers like vcpkg, Conan
- Versioning support through find_package
- Minimal CMakeLists.txt configuration

## Installation Locations

FlowGraph will be installed to standard locations:
- Headers: `/usr/local/include/`
- CMake config: `/usr/local/lib/cmake/FlowGraph/`
- Documentation: As specified by CMAKE_INSTALL_PREFIX