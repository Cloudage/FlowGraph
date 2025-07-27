# FlowGraph

[![CI](https://github.com/Cloudage/FlowGraph/actions/workflows/ci.yml/badge.svg)](https://github.com/Cloudage/FlowGraph/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

一个用于解析和执行基于文本的流程图的 C++ 头文件库。FlowGraph 为游戏和需要动态工作流功能的应用程序提供了一个简单、可嵌入的流程图执行引擎。

## 🚀 主要特性

- **头文件库**：只需要 `#include` 即可轻松集成
- **ExpressionKit 集成**：使用强大的 ExpressionKit 库进行完整的表达式求值
- **类型安全**：具有编译时验证的强类型系统
- **模块化**：每个 `.flow` 文件都是一个独立的、可重用的模块
- **可嵌入**：为游戏和应用程序的轻松集成而设计
- **跨平台**：支持 Windows、macOS、Linux 和移动平台
- **丰富的表达式支持**：算术运算、布尔逻辑、字符串操作和数学函数

## 快速开始

```cpp
#include "flowgraph/FlowGraph.hpp"

// 加载和执行流程
FlowGraph::Engine engine;
auto flow = engine.loadFlow("path/to/workflow.flow");
auto result = flow.execute();
```

## FlowGraph 格式

FlowGraph 使用简单的基于文本的格式来描述具有强大表达式支持的工作流：

```
TITLE: 用户认证

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
50 ASSIGN S message "登录 " + (success ? "成功" : "失败")

FLOW:
START -> 10
10 -> 20
20.Y -> 30
20.N -> 40
30 -> 40
40 -> 50
50 -> END
```

### 表达式支持

FlowGraph 集成了 [ExpressionKit](https://github.com/Cloudage/ExpressionKit) 进行完整的表达式求值：

**算术运算：**
- `2 + 3 * 4` → `14`
- `(a + b) / c` → 动态计算
- `-x` → 一元取反

**布尔逻辑：**
- `flag && (count > 5)` → 逻辑与
- `!active || ready` → 逻辑或与非
- `a > b && b <= c` → 比较链

**字符串操作：**
- `"你好，" + name + "！"` → 字符串连接
- `status == "active"` → 字符串比较

**数学函数：**
- `sqrt(16)` → `4`（平方根）
- `max(a, b)` → 最大值
- `min(x, y)` → 最小值
- `abs(-5)` → `5`（绝对值）
- `sin(3.14159/2)` → `1`（三角函数）

**变量访问：**
- 简单变量：`count`、`username`、`active`
- 点记法：`player.health`、`config.timeout`

完整规范请参阅 [FileFormat.md](FileFormat.md)。

## 安装

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

## 构建和测试

```bash
# 配置和构建
cmake -B build
cmake --build build

# 运行测试
ctest --test-dir build
```

## 项目结构

```
include/flowgraph/          # 头文件 C++ 库
├── FlowGraph.hpp          # 主头文件
├── Parser.hpp             # 流程文件解析器
├── Engine.hpp             # 执行引擎
├── Types.hpp              # 类型系统
└── Nodes.hpp              # 节点实现

tests/                     # C++ 单元测试（Catch2）
Sources/FlowGraph/         # Swift Package 包装器
Package.swift              # Swift Package 清单
Examples/                  # 使用示例
```

## 开发

该项目遵循测试驱动开发。开发路线图请参阅 [DEV_PHASE_1.cn.md](DEV_PHASE_1.cn.md)。

## 许可证

MIT 许可证 - 详情请参阅 [LICENSE](LICENSE)