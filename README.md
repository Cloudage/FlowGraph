# FlowGraph

[![CI](https://github.com/Cloudage/FlowGraph/actions/workflows/ci.yml/badge.svg)](https://github.com/Cloudage/FlowGraph/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A C++ header-only library for parsing and executing text-based flowcharts. FlowGraph provides a simple, embeddable flowchart execution engine designed for games and applications that need dynamic workflow capabilities.

## 🚀 Key Features

- **Header-only**: Easy integration with just `#include`
- **ExpressionKit Integration**: Complete expression evaluation using the powerful ExpressionKit library
- **Type-safe**: Strong typing system with compile-time validation
- **Modular**: Each `.flow` file is an independent, reusable module
- **Embeddable**: Designed for easy integration into games and applications
- **Cross-platform**: Works on Windows, macOS, Linux, and mobile platforms
- **Rich Expression Support**: Arithmetic, boolean logic, string operations, and mathematical functions

## Quick Start

```cpp
#include "flowgraph/FlowGraph.hpp"

// Load and execute a flow
FlowGraph::Engine engine;
auto flow = engine.loadFlow("path/to/workflow.flow");
auto result = flow.execute();
```

## FlowGraph Format

FlowGraph uses a simple text-based format for describing workflows with powerful expression support:

```
TITLE: User Authentication

PARAMS:
S username
S password

RETURNS:
B success
S token

NODES:
10 PROC check_user username>>login exists<<found
20 COND found
30 PROC generate_token username>>user token<<auth_token
40 ASSIGN B success true
50 ASSIGN S message "Login " + (success ? "successful" : "failed")

FLOW:
START -> 10
10 -> 20
20.Y -> 30
20.N -> 40
30 -> 40
40 -> 50
50 -> END
```

### Expression Support

FlowGraph integrates [ExpressionKit](https://github.com/Cloudage/ExpressionKit) for complete expression evaluation:

**Arithmetic Operations:**
- `2 + 3 * 4` → `14`
- `(a + b) / c` → Dynamic calculation
- `-x` → Unary negation

**Boolean Logic:**
- `flag && (count > 5)` → Logical AND
- `!active || ready` → Logical OR with NOT
- `a > b && b <= c` → Comparison chains

**String Operations:**
- `"Hello, " + name + "!"` → String concatenation
- `status == "active"` → String comparison

**Mathematical Functions:**
- `sqrt(16)` → `4` (Square root)
- `max(a, b)` → Maximum value
- `min(x, y)` → Minimum value
- `abs(-5)` → `5` (Absolute value)
- `sin(3.14159/2)` → `1` (Trigonometric functions)

**Variable Access:**
- Simple variables: `count`, `username`, `active`
- Dot notation: `player.health`, `config.timeout`

See [FileFormat.md](FileFormat.md) for the complete specification.

## Installation

### CMake

```cmake
include(FetchContent)
FetchContent_Declare(
  FlowGraph
  GIT_REPOSITORY https://github.com/Cloudage/FlowGraph.git
  GIT_TAG main
)
FetchContent_MakeAvailable(FlowGraph)

target_link_libraries(your_target FlowGraph::FlowGraph)
```

### Swift Package Manager

```swift
dependencies: [
    .package(url: "https://github.com/Cloudage/FlowGraph.git", from: "1.0.0")
]
```

## Building and Testing

```bash
# Configure and build
cmake -B build
cmake --build build

# Run tests
ctest --test-dir build
```

## Project Structure

```
include/flowgraph/          # Header-only C++ library
├── FlowGraph.hpp          # Main header
├── Parser.hpp             # Flow file parser
├── Engine.hpp             # Execution engine
├── Types.hpp              # Type system
└── Nodes.hpp              # Node implementations

tests/                     # C++ unit tests (Catch2)
Sources/FlowGraph/         # Swift Package wrapper
Package.swift              # Swift Package manifest
Examples/                  # Usage examples
```

## Development

This project follows test-driven development. See [DEV_PHASE_1.md](DEV_PHASE_1.md) for the development roadmap.

## License

MIT License - see [LICENSE](LICENSE) for details.