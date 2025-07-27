# FlowGraph Standalone Integration Example

This example shows how to use FlowGraph by manually copying headers from a generated package.

## Usage

1. Ensure you have a FlowGraph package generated:
   ```bash
   cd ../../../
   cmake -B build
   cmake --build build --target create_package
   ```

2. Compile and run the example:
   ```bash
   g++ -std=c++17 -I../../../build/flowgraph-1.0.0/include main.cpp -o standalone_example
   ./standalone_example
   ```

## What This Example Demonstrates

- Creating a FlowGraph engine
- Basic ExpressionKit integration 
- Creating AST nodes
- Self-contained header usage (no CMake required)

## Integration Method

This approach is ideal when:
- You want to avoid CMake build dependencies
- You need a simple, self-contained solution
- You're integrating into an existing build system
- You want to minimize external dependencies

## Files Included

- `main.cpp` - Example application
- This README with usage instructions

The headers are expected to be in a generated package at `../../../build/flowgraph-1.0.0/include/`.