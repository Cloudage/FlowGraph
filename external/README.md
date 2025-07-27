# ExpressionKit Integration Strategy

This directory currently contains a copy of ExpressionKit.hpp. This is a transitional approach.

## Recommended Migration Path

Once ExpressionKit is available as a proper CMake package or Git repository, this integration should be updated to use CMake's FetchContent:

```cmake
FetchContent_Declare(
    ExpressionKit
    GIT_REPOSITORY https://github.com/actual-org/ExpressionKit.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(ExpressionKit)
```

Then the copied header file (ExpressionKit.hpp) can be removed from this directory.

## Current Status

- FlowGraph now uses ExpressionKit::Value directly (no more FlowGraph::Value duplication)
- Conversion functions have been eliminated
- All integration is through the unified ExpressionKit::Value type
- Ready for proper FetchContent integration when ExpressionKit repository is available