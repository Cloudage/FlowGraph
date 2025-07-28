# FlowGraph 文件格式设计文档 v1.2

## 概述

FlowGraph 是一种基于文本的流程图描述格式，用于以人类可读和机器可解析的方式表示流程图。该格式将流程图视为有向图，支持模块化设计和类型安全的流程间调用。

## 设计原则

1. **简洁性**：只保留必要的流程图元素，避免冗余
1. **模块化**：每个文件是一个独立的流程模块，可被其他流程调用
1. **类型安全**：使用强类型系统，支持编译时类型检查
1. **可读性**：文本格式清晰，支持注释，自成文档
1. **图的本质**：流程图是有向图，循环通过边的连接自然形成

## 文件结构

```
/* 
 * 文件级注释（可选）
 * 可以跨越多行
 */
// 或者使用单行注释
TITLE: <流程名称>

PARAMS:    // 可选部分，如果没有参数可以省略
/* 参数定义部分的注释 */
// 参数注释
<类型> <参数名> [?]
...

RETURNS:   // 可选部分，如果没有返回值可以省略
// 返回值注释
<类型> <返回值名>
...

ERRORS:    // 可选部分，如果没有错误定义可以省略
// 错误定义注释
<错误名称>
...

NODES:
/* 节点定义部分 */
// 节点注释
<节点ID> <节点类型> <参数...>
...

FLOW:
START -> <节点ID>
<起始节点ID>[.端口] -> <目标节点ID>[.端口]
...
<节点ID> -> END
```

### 最简 FlowGraph 示例

无参数无返回值的流程可以非常简洁：

```
TITLE: 打印欢迎信息

NODES:
10 PROC print msg>>"欢迎使用系统"

FLOW:
START -> 10
10 -> END
```

## 注释规范

FlowGraph 支持两种注释格式：

### 单行注释

```
// 这是单行注释
```

### 多行注释

```
/*
 * 这是多行注释
 * 可以跨越多行
 * 常用于文件头部或复杂说明
 */
```

注释规则：

- 注释描述其后的第一个有效元素
- 空行会中断注释与元素的关联
- 多行注释不能嵌套

## 错误处理

FlowGraph 支持声明式错误处理机制，允许在流程中定义、捕获和发出错误。

### 错误定义

在文件头部使用 ERRORS 段声明错误：

```
ERRORS:
SOME_ERR
SOME_OTHER_ERROR
```

### 错误捕获

针对 PROC 节点，可以捕获特定错误并重定向流程：

```
30.SOME_ERR -> END
```

这表示如果节点 30（PROC 节点）抛出 SOME_ERR 错误，流程将跳转到 END。

### 错误发出

可以在任何节点发出自定义错误：

```
50.N -> SOME_ERROR
```

这表示如果条件节点 50 的结果为假（N），将发出 SOME_ERROR 错误。

## 节点类型

### 1. PROC - 处理节点

调用过程或其他 FlowGraph 文件。

语法：

```
PROC <过程名> [变量>>参数]* [变量<<返回值]*
```

示例：

```
// 调用函数
PROC validate_input data>>input errors<<result

// 调用其他 FlowGraph 文件
PROC auth/login.flow user>>username pass>>password success<<auth_ok token<<auth_token
```

### 2. ASSIGN - 赋值节点

为变量赋值，必须指定类型。

语法：

```
ASSIGN <类型> <变量名> <表达式>
```

类型：

- `N` - 数字 (Number) - 统一的数字类型，支持整数和浮点数
- `B` - 布尔值 (Boolean)
- `S` - 字符串 (String)

注意：为了向后兼容，旧的 `I` (整数) 和 `F` (浮点数) 类型标识符仍然可以使用，但都会被解析为统一的 `N` (数字) 类型。

示例：

```
ASSIGN N count 0          // 新的统一数字类型
ASSIGN N price 99.99      // 支持整数和浮点数
ASSIGN B is_valid true
ASSIGN S message "Hello, " + username

// 向后兼容示例（仍然有效）
ASSIGN I old_count 0      // 旧的整数类型，解析为数字类型
ASSIGN F old_price 99.99  // 旧的浮点类型，解析为数字类型
```

### 3. COND - 条件节点

条件判断，有两个出口：Y（真）和 N（假）。

语法：

```
COND <条件表达式>
```

示例：

```
COND count < max_attempts
COND username != null && password != null
```

## 流程定义

### START 和 END

- `START` - 流程入口，每个流程必须有且只有一个
- `END` - 流程出口，可以有多个

### 连接语法

- 默认连接：`10 -> 20`
- 条件分支：`30.Y -> 40` 或 `30.N -> 50`

## 参数系统

### 参数定义（可选）

```
PARAMS:
// 必需参数
<类型> <参数名>
// 可选参数（带 ? 后缀）
<类型> <参数名> ?
```

### 返回值定义（可选）

```
RETURNS:
<类型> <返回值名>
```

### 参数传递

- 使用 `>>` 传出数据：`local_var>>param_name`
- 使用 `<<` 接收数据：`local_var<<return_name`
- 调用时参数顺序无关，通过名称匹配

## 完整示例

### 带参数和返回值的 FlowGraph

```
/*
 * 用户认证 FlowGraph
 * 
 * 功能：验证用户身份并生成访问令牌
 * 作者：系统架构组
 * 版本：1.0
 */
TITLE: 用户登录验证

PARAMS:
/* 登录凭据 */
// 用户登录名（支持用户名或邮箱）
S username
// 用户密码（明文传入，内部加密）
S password

RETURNS:
// 是否登录成功
B success
/* 
 * 访问令牌
 * 成功时返回有效的 JWT token
 * 失败时返回空字符串
 */
S token
// 错误信息（成功时为空）
S error_msg

NODES:
/* 验证流程开始 */
// 查询用户是否存在
20 PROC check_user username>>login exists<<found hash<<pwd_hash
30 COND found

/* 密码验证部分 */
// 使用 bcrypt 验证密码
40 PROC verify_password password>>input pwd_hash>>hash match<<is_valid
50 COND is_valid

// 生成 JWT 令牌
60 PROC generate_token username>>user token<<auth_token
70 ASSIGN B success true
80 ASSIGN S error_msg ""

/* 错误处理 */
// 统一的失败处理
100 ASSIGN B success false
110 ASSIGN S token ""
120 ASSIGN S error_msg "用户名或密码错误"

FLOW:
START -> 20
20 -> 30
30.Y -> 40
30.N -> 100
40 -> 50
50.Y -> 60
50.N -> 100
60 -> 70
70 -> 80
80 -> END
100 -> 110
110 -> 120
120 -> END
```

### 无参数无返回值的 FlowGraph

```
// 系统初始化流程
TITLE: 系统启动初始化

NODES:
10 PROC load_config config<<conf
20 PROC init_database config>>conf success<<db_ok
30 COND db_ok
40 PROC start_services
50 PROC log msg>>"系统启动成功"
60 PROC log msg>>"数据库初始化失败"
70 PROC shutdown

FLOW:
START -> 10
10 -> 20
20 -> 30
30.Y -> 40
30.N -> 60
40 -> 50
50 -> END
60 -> 70
70 -> END
```

### 只有返回值的 FlowGraph

```
/* 获取系统状态信息 */
TITLE: 系统状态检查

RETURNS:
// CPU使用率（0-100）
I cpu_usage
// 内存使用率（0-100）
I mem_usage
// 系统是否健康
B is_healthy

NODES:
10 PROC get_cpu_stats usage<<cpu_usage
20 PROC get_memory_stats usage<<mem_usage
30 ASSIGN B is_healthy cpu_usage < 80 && mem_usage < 90

FLOW:
START -> 10
10 -> 20
20 -> 30
30 -> END
```

### 带错误处理的 FlowGraph

```
/*
 * 用户认证流程 - 带错误处理
 * 
 * 演示错误定义、捕获和发出
 */
TITLE: 用户认证（带错误处理）

PARAMS:
S username
S password

RETURNS:
B success
S token

ERRORS:
USER_NOT_FOUND
INVALID_PASSWORD
AUTH_SERVICE_ERROR

NODES:
/* 验证流程 */
10 PROC check_user username>>login exists<<found
20 COND found
30 PROC verify_password password>>input username>>user valid<<is_valid
40 COND is_valid
50 PROC generate_token username>>user token<<auth_token
60 ASSIGN B success true

/* 错误处理 */
100 ASSIGN B success false
110 ASSIGN S token ""

FLOW:
START -> 10
10 -> 20
10.USER_NOT_FOUND -> 100    // 捕获用户不存在错误
20.Y -> 30
20.N -> USER_NOT_FOUND      // 发出用户不存在错误
30 -> 40
30.AUTH_SERVICE_ERROR -> 100  // 捕获认证服务错误
40.Y -> 50
40.N -> INVALID_PASSWORD    // 发出密码错误
50 -> 60
60 -> END
100 -> 110
110 -> END
```

## 文件扩展名

FlowGraph 文件使用 `.flow` 扩展名。

## 文件组织建议

```
project/
├── main.flow              # 主 FlowGraph 入口
├── auth/
│   ├── login.flow        # 登录 FlowGraph
│   ├── register.flow     # 注册 FlowGraph
│   └── reset.flow        # 密码重置 FlowGraph
├── data/
│   ├── validate.flow     # 数据验证 FlowGraph
│   └── process.flow      # 数据处理 FlowGraph
└── utils/
    ├── email.flow        # 邮件发送 FlowGraph
    └── log.flow          # 日志记录 FlowGraph
```

## 表达式支持

支持的运算符：

- 算术：`+`, `-`, `*`, `/`, `%`
- 比较：`==`, `!=`, `>`, `<`, `>=`, `<=`
- 逻辑：`&&`, `||`, `!`
- 字符串连接：`+`

## 设计优势

1. **人机可读**：文本格式便于版本控制和代码审查
1. **类型安全**：强类型系统预防运行时错误
1. **模块化**：每个 FlowGraph 可独立开发、测试和重用
1. **自文档化**：通过注释和类型信息自动生成文档
1. **工具友好**：易于开发解析器、验证器和可视化工具

## 工具生态

建议的 FlowGraph 工具集：

- **解析器**：将 .flow 文件解析为 AST
- **验证器**：类型检查和流程完整性验证
- **可视化器**：将 FlowGraph 渲染为图形
- **执行引擎**：解释执行 FlowGraph
- **转换器**：转换为其他格式（如 Mermaid、GraphViz）

## 扩展性

FlowGraph 设计为核心功能集，可根据需要扩展：

- 添加新的数据类型（如数组、对象）
- 支持异常处理机制
- 添加并行处理节点
- 支持子流程内联定义
- 添加断言和调试节点​​​​​​​​​​​​​​​​
