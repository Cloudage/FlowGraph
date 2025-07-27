# FlowGraph Development Phase 1

## Overview

Phase 1 focuses on creating a minimal viable FlowGraph library that can parse and execute basic flowcharts. This phase establishes the foundation for the entire system.

## Goals

1. **Core Parsing**: Parse `.flow` files according to the specification
2. **Basic Execution**: Execute simple flowcharts with all node types
3. **Expression Evaluation**: Integrate Cloudage/ExpressionKit for expressions
4. **Type Safety**: Implement strong typing for parameters and variables
5. **Test Coverage**: Comprehensive unit tests using Catch2
6. **Swift Wrapper**: Basic Swift Package with C++ interop

## Architecture Design

### Core Components

```
FlowGraph Library Architecture
├── Parser (FlowGraph::Parser)
│   ├── Lexer - Tokenize .flow files
│   ├── Syntax Parser - Build AST from tokens
│   └── Semantic Analyzer - Type checking and validation
├── Engine (FlowGraph::Engine)
│   ├── Flow Loader - Load and cache flows
│   ├── Executor - Execute flow graphs
│   └── Context Manager - Variable and scope management
├── Type System (FlowGraph::Types)
│   ├── Value - Runtime value representation
│   ├── TypeInfo - Compile-time type information
│   └── TypeChecker - Type validation
└── Nodes (FlowGraph::Nodes)
    ├── ProcNode - Procedure/function calls
    ├── AssignNode - Variable assignments
    ├── CondNode - Conditional branching
    ├── StartNode - Flow entry point
    └── EndNode - Flow exit point
```

### File Organization

```
include/flowgraph/
├── FlowGraph.hpp           # Main public API header
├── detail/                 # Implementation details
│   ├── Parser.hpp          # Parser implementation
│   ├── Engine.hpp          # Execution engine
│   ├── Types.hpp           # Type system
│   ├── Nodes.hpp           # Node implementations
│   ├── Context.hpp         # Execution context
│   └── AST.hpp             # Abstract syntax tree
├── FlowGraphConfig.hpp     # Configuration options
└── FlowGraphVersion.hpp    # Version information

tests/
├── unit/                   # Unit tests
│   ├── test_parser.cpp     # Parser tests
│   ├── test_engine.cpp     # Engine tests
│   ├── test_types.cpp      # Type system tests
│   └── test_nodes.cpp      # Node tests
├── integration/            # Integration tests
│   ├── test_basic_flows.cpp # Basic flow execution
│   └── test_examples.cpp   # Example flows
└── test_main.cpp           # Catch2 test runner

examples/
├── basic/                  # Basic usage examples
│   ├── hello.flow          # Simple hello world
│   ├── calculator.flow     # Basic calculator
│   └── user_auth.flow      # User authentication
└── advanced/               # Advanced examples
    ├── game_logic.flow     # Game state machine
    └── data_pipeline.flow  # Data processing pipeline
```

## Implementation Plan

### Milestone 1: Project Setup (Week 1)
- [ ] CMake build system setup
- [ ] ExpressionKit integration via FetchContent
- [ ] Catch2 testing framework setup
- [ ] Basic project structure creation
- [ ] CI/CD pipeline (GitHub Actions)

### Milestone 2: Core Parser (Week 2-3)
- [ ] Lexer implementation
  - [ ] Token definitions (keywords, operators, literals)
  - [ ] Comment handling (single-line and multi-line)
  - [ ] Error reporting with line/column information
- [ ] Parser implementation
  - [ ] TITLE, PARAMS, RETURNS sections
  - [ ] NODES section with all node types
  - [ ] FLOW section with connections
  - [ ] AST generation
- [ ] Parser tests with comprehensive edge cases

### Milestone 3: Type System (Week 4)
- [ ] Value type implementation (I, F, B, S)
- [ ] Type checking and validation
- [ ] Parameter and return value type matching
- [ ] Expression type inference
- [ ] Type system tests

### Milestone 4: Execution Engine (Week 5-6)
- [ ] Execution context management
- [ ] Variable storage and scoping
- [ ] Node execution framework
- [ ] Flow control (START, END, branching)
- [ ] Basic node implementations:
  - [ ] ASSIGN nodes
  - [ ] COND nodes
  - [ ] Basic PROC nodes (built-in functions)
- [ ] Engine tests

### Milestone 5: Expression Integration (Week 7)
- [ ] ExpressionKit integration
- [ ] Variable resolution in expressions
- [ ] Type-safe expression evaluation
- [ ] Expression error handling
- [ ] Expression tests

### Milestone 6: Advanced Features (Week 8)
- [ ] External PROC node support (calling other .flow files)
- [ ] Parameter passing between flows
- [ ] Error propagation and handling
- [ ] Performance optimizations
- [ ] Integration tests

### Milestone 7: Swift Package (Week 9)
- [ ] Swift Package structure
- [ ] C++ interop layer
- [ ] Swift-friendly API design
- [ ] Swift example and tests
- [ ] Documentation

### Milestone 8: Polish and Documentation (Week 10)
- [ ] Complete API documentation
- [ ] Performance benchmarks
- [ ] Example applications
- [ ] User guide and tutorials
- [ ] Security review

## Test-Driven Development Approach

### Test Categories

1. **Unit Tests**: Test individual components in isolation
   - Parser components (lexer, syntax parser)
   - Type system components
   - Individual node types
   - Engine components

2. **Integration Tests**: Test component interactions
   - Parser → AST → Engine pipeline
   - Flow execution with multiple node types
   - Expression evaluation integration
   - Error handling across components

3. **Example Tests**: Test real-world usage scenarios
   - All examples in examples/ directory must have tests
   - Performance benchmarks
   - Memory usage validation

### Test Structure

Each test file follows this pattern:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "flowgraph/FlowGraph.hpp"

TEST_CASE("Component description", "[component][category]") {
    SECTION("specific scenario") {
        // Arrange
        // Act  
        // Assert
    }
}
```

## Dependencies

### Required Dependencies
- **Cloudage/ExpressionKit**: Expression parsing and evaluation
- **Catch2**: Unit testing framework (test-only)

### Build Dependencies
- **CMake 3.15+**: Build system
- **C++17 compiler**: GCC 7+, Clang 6+, MSVC 2019+

## Error Handling Strategy

### Parser Errors
- Syntax errors with precise location information
- Semantic errors (type mismatches, undefined references)
- Recoverable vs. fatal error classification

### Runtime Errors
- Type conversion errors
- Expression evaluation errors
- Flow execution errors (infinite loops, missing nodes)
- External resource errors (file not found, permission denied)

### Error Reporting
```cpp
class FlowGraphError : public std::exception {
public:
    enum class Type { Parse, Type, Runtime, IO };
    
    Type type() const noexcept;
    std::string_view message() const noexcept;
    std::optional<Location> location() const noexcept;
};
```

## Performance Considerations

### Design Goals
- **Parse Time**: < 1ms for typical flows (< 100 nodes)
- **Memory Usage**: < 1MB per loaded flow
- **Execution Speed**: > 10,000 node executions/second
- **Startup Time**: < 10ms for library initialization

### Optimization Strategies
- AST caching for frequently used flows
- Expression pre-compilation
- Node execution optimizations
- Memory pool allocation for runtime values

## API Design Principles

### C++ API
- Header-only for easy integration
- RAII and exception-safe
- Modern C++17 features (std::optional, std::string_view)
- Minimal public API surface

### Swift API
- Swifty naming conventions
- Error handling with Result types
- Value semantics where appropriate
- Memory management handled automatically

## Risk Assessment

### High Risk
- ExpressionKit integration complexity
- Performance requirements for real-time applications
- Swift interop stability

### Medium Risk  
- Parser robustness with edge cases
- Memory management in long-running applications
- Cross-platform compatibility

### Low Risk
- Basic functionality implementation
- Test coverage and quality
- Documentation completeness

## Success Criteria

Phase 1 is complete when:
1. All basic node types (PROC, ASSIGN, COND) work correctly
2. Parser handles all syntax from FileFormat.md specification
3. Expression evaluation works through ExpressionKit
4. Test coverage > 90% for core functionality
5. Swift Package builds and basic interop works
6. Example flows execute correctly
7. Performance meets minimum requirements
8. Documentation is complete and accurate

## Next Phases Preview

- **Phase 2**: Advanced features (sub-flows, error handling, debugging)
- **Phase 3**: Performance optimization and production features
- **Phase 4**: Visual tools and IDE integration
- **Phase 5**: Advanced language features and ecosystem