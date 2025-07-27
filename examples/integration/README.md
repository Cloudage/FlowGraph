# FlowGraph Integration Examples

This directory contains examples showing different ways to integrate FlowGraph into your project.

## Method 1: Manual Header Copy (Standalone Package)

The `standalone/` directory shows how to use FlowGraph after downloading a pre-built package.

### Steps:
1. Download a FlowGraph package (generated with `cmake --build build --target create_package`)
2. Copy the `include/` directory to your project
3. Include FlowGraph headers and compile with C++17

### Example:
```bash
cd standalone/
g++ -std=c++17 -I../../../build/flowgraph-1.0.0/include main.cpp -o standalone_example
./standalone_example
```

## Method 2: CMake FetchContent

The `fetchcontent/` directory shows how to use FlowGraph via CMake FetchContent.

### Steps:
1. Add FetchContent declaration to your CMakeLists.txt
2. Link against FlowGraph::FlowGraph target
3. Build normally with CMake

### Example:
```bash
cd fetchcontent/
cmake -B build
cmake --build build
./build/fetchcontent_example
```

## Method 3: CMake find_package (After Installation)

The `findpackage/` directory shows how to use FlowGraph after system installation.

### Steps:
1. Install FlowGraph system-wide (`cmake --install build`)
2. Use find_package(FlowGraph) in your CMakeLists.txt
3. Link against FlowGraph::FlowGraph target

### Example:
```bash
# First install FlowGraph (requires admin privileges)
cd ../../..
sudo cmake --install build

# Then use it
cd examples/integration/findpackage/
cmake -B build
cmake --build build
./build/findpackage_example
```

## Requirements

All methods require:
- C++17 compatible compiler
- CMake 3.15+ (for FetchContent and find_package methods)