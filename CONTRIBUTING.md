# Contributing to FlowGraph

Thank you for your interest in contributing to FlowGraph! This document provides guidelines for contributing to the project.

## Development Setup

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler (GCC 7+, Clang 6+, MSVC 2019+)
- Git

### Building

```bash
# Clone the repository
git clone https://github.com/Cloudage/FlowGraph.git
cd FlowGraph

# Configure and build
cmake -B build -DFLOWGRAPH_BUILD_TESTS=ON -DFLOWGRAPH_BUILD_EXAMPLES=ON
cmake --build build

# Run tests
ctest --test-dir build
```

### Project Structure

```
include/flowgraph/          # Header-only C++ library
├── FlowGraph.hpp          # Main public API
└── detail/                # Implementation details
    ├── Types.hpp          # Type system
    ├── AST.hpp            # Abstract syntax tree
    ├── Parser.hpp         # Flow file parser
    └── Engine.hpp         # Execution engine

tests/                     # Test suite
├── unit/                  # Unit tests
└── integration/           # Integration tests

Sources/                   # Swift Package
├── CFlowGraph/           # C wrapper for Swift interop
└── FlowGraph/            # Swift API

examples/                  # Usage examples
```

## Development Workflow

### Test-Driven Development

FlowGraph follows TDD principles:

1. Write a failing test that demonstrates the feature
2. Write minimal code to make the test pass
3. Refactor and improve the implementation
4. Ensure all tests still pass

### Code Style

- Follow existing code style and naming conventions
- Use C++17 features appropriately
- Keep the public API surface minimal
- Document public APIs with clear comments
- Prefer header-only implementation for ease of integration

### Testing

- All new features must have comprehensive tests
- Unit tests should test individual components in isolation
- Integration tests should test component interactions
- All tests must pass before submitting PRs

### Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes following TDD
4. Ensure all tests pass
5. Update documentation if needed
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

## Development Phases

Currently in **Phase 1** - see [DEV_PHASE_1.md](DEV_PHASE_1.md) for detailed roadmap.

### Current Priorities

1. Complete the lexer and parser implementation
2. Integrate Cloudage/ExpressionKit for expression evaluation
3. Implement the execution engine
4. Complete Swift Package implementation

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn and grow
- Keep discussions technical and objective

## Questions?

- Check existing issues for similar questions
- Create a new issue for bugs or feature requests
- Use discussions for general questions about usage

Thank you for contributing to FlowGraph!