# FlowGraph FetchContent Integration Example

This example shows how to use FlowGraph via CMake FetchContent.

## Usage

```bash
cmake -B build
cmake --build build
./build/fetchcontent_example
```

## What This Example Demonstrates

- Using FlowGraph via CMake FetchContent
- Automatic dependency resolution (ExpressionKit is fetched automatically)
- Proper CMake target linking
- Expression evaluation functionality
- AST node creation

## Integration Method

This approach is ideal when:
- You're using CMake for your project
- You want automatic dependency management
- You want to always use the latest version
- You prefer not to manage dependencies manually

## CMakeLists.txt Explanation

The key parts of the CMakeLists.txt:

```cmake
# Fetch FlowGraph
include(FetchContent)
FetchContent_Declare(
    FlowGraph
    GIT_REPOSITORY https://github.com/Cloudage/FlowGraph.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(FlowGraph)

# Link against FlowGraph
target_link_libraries(your_target PRIVATE FlowGraph::FlowGraph)
```

## Benefits

- Automatic dependency resolution
- Always up-to-date with repository
- CMake handles all include paths
- Works with any CMake-based project
- No manual header management required