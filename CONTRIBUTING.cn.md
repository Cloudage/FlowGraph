# 参与 FlowGraph 项目

感谢您对参与 FlowGraph 项目的兴趣！本文档为项目贡献提供指导原则。

## 开发环境设置

### 前置条件

- CMake 3.15 或更高版本
- C++17 兼容编译器（GCC 7+、Clang 6+、MSVC 2019+）
- Git

### 构建

```bash
# 克隆仓库
git clone https://github.com/Cloudage/FlowGraph.git
cd FlowGraph

# 配置和构建
cmake -B build -DFLOWGRAPH_BUILD_TESTS=ON -DFLOWGRAPH_BUILD_EXAMPLES=ON
cmake --build build

# 运行测试
ctest --test-dir build
```

### 项目结构

```
include/flowgraph/          # 头文件 C++ 库
├── FlowGraph.hpp          # 主要公共 API
└── detail/                # 实现细节
    ├── Types.hpp          # 类型系统
    ├── AST.hpp            # 抽象语法树
    ├── Parser.hpp         # 流程文件解析器
    └── Engine.hpp         # 执行引擎

tests/                     # 测试套件
├── unit/                  # 单元测试
└── integration/           # 集成测试

Sources/                   # Swift Package
├── CFlowGraph/           # Swift 互操作的 C 包装器
└── FlowGraph/            # Swift API

examples/                  # 使用示例
```

## 开发工作流

### 测试驱动开发

FlowGraph 遵循 TDD 原则：

1. 编写一个演示功能的失败测试
2. 编写最少的代码使测试通过
3. 重构和改进实现
4. 确保所有测试仍然通过

### 代码规范

- 遵循现有的代码风格和命名约定
- 适当使用 C++17 特性
- 保持公共 API 表面最小化
- 用清晰的注释记录公共 API
- 为了便于集成，优先使用头文件实现

### 测试

- 所有新功能必须有全面的测试
- 单元测试应该独立测试各个组件
- 集成测试应该测试组件交互
- 提交 PR 之前所有测试必须通过

### Pull Request 流程

1. Fork 仓库
2. 创建功能分支（`git checkout -b feature/amazing-feature`）
3. 遵循 TDD 进行更改
4. 确保所有测试通过
5. 如有需要更新文档
6. 提交更改（`git commit -m 'Add amazing feature'`）
7. 推送到分支（`git push origin feature/amazing-feature`）
8. 开启 Pull Request

## 开发阶段

目前处于**第一阶段** - 详细路线图请参阅 [DEV_PHASE_1.cn.md](DEV_PHASE_1.cn.md)。

### 当前优先级

1. 完成词法分析器和解析器实现
2. 集成 Cloudage/ExpressionKit 进行表达式求值
3. 实现执行引擎
4. 完成 Swift Package 实现

## 行为准则

- 保持尊重和包容
- 专注于建设性反馈
- 帮助他人学习和成长
- 保持讨论技术性和客观性

## 问题？

- 查看现有 issue 是否有类似问题
- 为 bug 或功能请求创建新 issue
- 使用讨论区询问使用相关的一般问题

感谢您参与 FlowGraph 项目！