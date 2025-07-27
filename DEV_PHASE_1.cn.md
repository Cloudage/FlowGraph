# FlowGraph 开发第一阶段

## 概述

第一阶段专注于创建一个最小可行的 FlowGraph 库，能够解析和执行基本的流程图。这个阶段为整个系统奠定基础。

## 目标

1. **核心解析**：根据规范解析 `.flow` 文件
2. **基础执行**：执行包含所有节点类型的简单流程图
3. **表达式求值**：集成 Cloudage/ExpressionKit 处理表达式
4. **类型安全**：为参数和变量实现强类型
5. **测试覆盖**：使用 Catch2 进行全面单元测试
6. **Swift 包装器**：具有 C++ 互操作的基本 Swift Package

## 架构设计

### 核心组件

```
FlowGraph 库架构
├── Parser (FlowGraph::Parser)
│   ├── Lexer - 对 .flow 文件进行词法分析
│   ├── Syntax Parser - 从 token 构建 AST
│   └── Semantic Analyzer - 类型检查和验证
├── Engine (FlowGraph::Engine)
│   ├── Flow Loader - 加载和缓存流程
│   ├── Executor - 执行流程图
│   └── Context Manager - 变量和作用域管理
├── Type System (FlowGraph::Types)
│   ├── Value - 运行时值表示
│   ├── TypeInfo - 编译时类型信息
│   └── TypeChecker - 类型验证
└── Nodes (FlowGraph::Nodes)
    ├── ProcNode - 过程/函数调用
    ├── AssignNode - 变量赋值
    ├── CondNode - 条件分支
    ├── StartNode - 流程入口点
    └── EndNode - 流程出口点
```

### 文件组织

```
include/flowgraph/
├── FlowGraph.hpp           # 主要公共 API 头文件
├── detail/                 # 实现细节
│   ├── Parser.hpp          # 解析器实现
│   ├── Engine.hpp          # 执行引擎
│   ├── Types.hpp           # 类型系统
│   ├── Nodes.hpp           # 节点实现
│   ├── Context.hpp         # 执行上下文
│   └── AST.hpp             # 抽象语法树
├── FlowGraphConfig.hpp     # 配置选项
└── FlowGraphVersion.hpp    # 版本信息

tests/
├── unit/                   # 单元测试
│   ├── test_parser.cpp     # 解析器测试
│   ├── test_engine.cpp     # 引擎测试
│   ├── test_types.cpp      # 类型系统测试
│   └── test_nodes.cpp      # 节点测试
├── integration/            # 集成测试
│   ├── test_basic_flows.cpp # 基础流程执行
│   └── test_examples.cpp   # 示例流程
└── test_main.cpp           # Catch2 测试运行器

examples/
├── basic/                  # 基础使用示例
│   ├── hello.flow          # 简单的 hello world
│   ├── calculator.flow     # 基础计算器
│   └── user_auth.flow      # 用户认证
└── advanced/               # 高级示例
    ├── game_logic.flow     # 游戏状态机
    └── data_pipeline.flow  # 数据处理管道
```

## 实现计划

### 里程碑 1：项目设置（第 1 周）
- [ ] CMake 构建系统设置
- [ ] 通过 FetchContent 集成 ExpressionKit
- [ ] Catch2 测试框架设置
- [ ] 基础项目结构创建
- [ ] CI/CD 管道（GitHub Actions）

### 里程碑 2：核心解析器（第 2-3 周）
- [ ] 词法分析器实现
  - [ ] Token 定义（关键字、操作符、字面量）
  - [ ] 注释处理（单行和多行）
  - [ ] 带行/列信息的错误报告
- [ ] 解析器实现
  - [ ] TITLE、PARAMS、RETURNS 部分
  - [ ] NODES 部分包含所有节点类型
  - [ ] FLOW 部分包含连接
  - [ ] AST 生成
- [ ] 解析器测试包含全面的边界情况

### 里程碑 3：类型系统（第 4 周）
- [ ] 值类型实现（I、F、B、S）
- [ ] 类型检查和验证
- [ ] 参数和返回值类型匹配
- [ ] 表达式类型推断
- [ ] 类型系统测试

### 里程碑 4：执行引擎（第 5-6 周）
- [ ] 执行上下文管理
- [ ] 变量存储和作用域
- [ ] 节点执行框架
- [ ] 流程控制（START、END、分支）
- [ ] 基本节点实现：
  - [ ] ASSIGN 节点
  - [ ] COND 节点
  - [ ] 基本 PROC 节点（内置函数）
- [ ] 引擎测试

### 里程碑 5：表达式集成（第 7 周）
- [ ] ExpressionKit 集成
- [ ] 表达式中的变量解析
- [ ] 类型安全的表达式求值
- [ ] 表达式错误处理
- [ ] 表达式测试

**ExpressionKit 集成遇到的挑战：**

在集成 Cloudage/ExpressionKit 进行表达式求值期间，识别出了几个关键挑战：

1. **字符串类型支持**：ExpressionKit 目前仅支持数值（int64_t、double）和布尔类型。FlowGraph 需要字符串类型支持用于基于文本的条件、参数传递和动态文本生成。此限制要求我们：
   - 向 ExpressionKit 提交原生字符串支持的功能请求
   - 考虑字符串操作的临时解决方案
   - 规划字符串支持可用时的未来迁移

2. **类型系统对齐**：FlowGraph 的 4 类型系统（整数、浮点、布尔、字符串）需要与 ExpressionKit 当前的 2 类型系统（数字、布尔）对齐。这为以下方面带来了挑战：
   - 系统间的类型强制转换
   - 错误消息一致性
   - 类型转换的性能

3. **变量环境集成**：ExpressionKit 的 IEnvironment 接口需要与 FlowGraph 的 ExecutionContext 变量存储桥接，需要仔细考虑：
   - 变量作用域规则
   - 桥接的类型安全
   - 变量查找的性能

4. **Swift Package 依赖项**：将 ExpressionKit 作为 Swift Package 的依赖项会增加以下复杂性：
   - 跨平台编译
   - C++ 互操作层管理
   - 包版本兼容性

这些挑战正在通过以下方式解决：
- 向 ExpressionKit 提交字符串支持的功能请求
- 仔细设计类型系统之间的桥接
- 全面测试集成层
- 记录解决方案和未来迁移路径

### 里程碑 6：高级功能（第 8 周）
- [ ] 外部 PROC 节点支持（调用其他 .flow 文件）
- [ ] 流程间参数传递
- [ ] 错误传播和处理
- [ ] 性能优化
- [ ] 集成测试

### 里程碑 7：Swift Package（第 9 周）
- [ ] Swift Package 结构
- [ ] C++ 互操作层
- [ ] Swift 友好的 API 设计
- [ ] Swift 示例和测试
- [ ] 文档

### 里程碑 8：完善和文档（第 10 周）
- [ ] 完整的 API 文档
- [ ] 性能基准测试
- [ ] 示例应用程序
- [ ] 用户指南和教程
- [ ] 安全审查

## 测试驱动开发方法

### 测试类别

1. **单元测试**：独立测试各个组件
   - 解析器组件（词法分析器、语法解析器）
   - 类型系统组件
   - 各个节点类型
   - 引擎组件

2. **集成测试**：测试组件交互
   - 解析器 → AST → 引擎管道
   - 多节点类型的流程执行
   - 表达式求值集成
   - 跨组件错误处理

3. **示例测试**：测试实际使用场景
   - examples/ 目录中的所有示例必须有测试
   - 性能基准测试
   - 内存使用验证

### 测试结构

每个测试文件遵循以下模式：
```cpp
#include <catch2/catch_test_macros.hpp>
#include "flowgraph/FlowGraph.hpp"

TEST_CASE("组件描述", "[component][category]") {
    SECTION("特定场景") {
        // Arrange（准备）
        // Act（执行）  
        // Assert（断言）
    }
}
```

## 依赖项

### 必需依赖项
- **Cloudage/ExpressionKit**：表达式解析和求值
- **Catch2**：单元测试框架（仅测试）

### 构建依赖项
- **CMake 3.15+**：构建系统
- **C++17 编译器**：GCC 7+、Clang 6+、MSVC 2019+

## 错误处理策略

### 解析器错误
- 带精确位置信息的语法错误
- 语义错误（类型不匹配、未定义引用）
- 可恢复 vs. 致命错误分类

### 运行时错误
- 类型转换错误
- 表达式求值错误
- 流程执行错误（无限循环、缺失节点）
- 外部资源错误（文件未找到、权限拒绝）

### 错误报告
```cpp
class FlowGraphError : public std::exception {
public:
    enum class Type { Parse, Type, Runtime, IO };
    
    Type type() const noexcept;
    std::string_view message() const noexcept;
    std::optional<Location> location() const noexcept;
};
```

## 性能考虑

### 设计目标
- **解析时间**：典型流程（< 100 个节点）< 1ms
- **内存使用**：每个加载的流程 < 1MB
- **执行速度**：> 10,000 节点执行/秒
- **启动时间**：库初始化 < 10ms

### 优化策略
- 常用流程的 AST 缓存
- 表达式预编译
- 节点执行优化
- 运行时值的内存池分配

## API 设计原则

### C++ API
- 头文件库便于集成
- RAII 和异常安全
- 现代 C++17 特性（std::optional、std::string_view）
- 最小化公共 API 表面

### Swift API
- Swifty 命名约定
- 使用 Result 类型进行错误处理
- 适当使用值语义
- 自动处理内存管理

## 风险评估

### 高风险
- ExpressionKit 集成复杂性
- 实时应用程序的性能要求
- Swift 互操作稳定性

### 中等风险  
- 解析器对边界情况的鲁棒性
- 长时间运行应用程序中的内存管理
- 跨平台兼容性

### 低风险
- 基本功能实现
- 测试覆盖率和质量
- 文档完整性

## 成功标准

第一阶段在以下条件下完成：
1. 所有基本节点类型（PROC、ASSIGN、COND）正常工作
2. 解析器处理来自 FileFormat.md 规范的所有语法
3. 表达式求值通过 ExpressionKit 工作
4. 核心功能的测试覆盖率 > 90%
5. Swift Package 构建且基本互操作工作
6. 示例流程正确执行
7. 性能满足最低要求
8. 文档完整准确

## 下一阶段预览

- **第二阶段**：高级功能（子流程、错误处理、调试）
- **第三阶段**：性能优化和生产功能
- **第四阶段**：可视化工具和 IDE 集成
- **第五阶段**：高级语言功能和生态系统