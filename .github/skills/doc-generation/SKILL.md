---
name: doc-generation
description: "为 C 或 Python 代码生成中文注释、函数说明、模块文档。当用户请求添加注释、生成文档、解释代码、写函数说明时使用。"
---

# 代码文档生成

本项目包含 C（Vitis PS端）、Python/PyQt5（上位机）、Qt C++（上位机）、Verilog（PL端）四类代码，注释均使用**中文**。

## 步骤

1. 阅读目标文件，判断语言类型（C、Python、Qt C++、Verilog）
2. 识别所有 public 函数/方法/模块（C 中无 `static`、Python 中无前置 `_`）
3. 为每个函数/模块生成标准注释
4. 在文件头添加模块说明

## C 代码注释风格

### 文件头注释

```c
/*
 * 文件 : 文件名.c
 * 描述 : 模块简要说明
 * 版本 : v1.0
 * 日期 : YYYY-MM-DD
 *
 * 修改记录（最新版本在最前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   xxx      YY/MM/DD   创建文件
 */
```

### 函数注释

```c
/**
 * @brief  函数简要说明
 * @param  param1  参数1说明
 * @param  param2  参数2说明
 * @return 返回值说明，或 void
 *
 * @note   注意事项（可选）
 */
```

### 行内注释

- 解释非显而易见的逻辑，不解释显然的代码
- 重要分支/寄存器操作必须注释
- 标记待办：`/* TODO: 说明 */`
- 标记新增修改：`/* YYYY-MM-DD 新增：说明 */`

## Python 代码注释风格

### 函数注释（docstring）

```python
def func_name(param1, param2):
    """
    函数简要说明。

    Args:
        param1: 参数1说明
        param2: 参数2说明

    Returns:
        返回值说明
    """
```

### 类注释

```python
class MyClass:
    """
    类的用途说明。

    Attributes:
        attr1: 属性1说明
    """
```

## Qt C++ 注释风格（qt/）

### 文件头注释

修改记录**最新版本在最前**（倒序插入）：

```cpp
/**
 * @file   xxx.cpp
 * @brief  模块简要说明
 * @version v1.1
 * @date    YYYY-MM-DD
 *
 * 修改记录（最新版本在最前）:
 *   vX.Y  YYYY-MM-DD  新增/修复/调整 说明
 *   v1.0  YYYY-MM-DD  创建文件
 */
```

### 函数注释

```cpp
/**
 * @brief  函数简要说明
 * @param  param1  参数1说明
 * @param  param2  参数2说明
 * @return 返回值说明
 * @note   注意事项（可选）
 */
```

### 结构体/类成员注释

使用 `///< ` 行尾注释：

```cpp
struct PendingEntry {
    qint64   sentAt;    ///< 发送时间戳（毫秒）
    QString  label;     ///< 命令描述（用于日志）
    bool     silentAck; ///< 是否静默 ACK
};
```

## doc/ Markdown 文档版本控制

`doc/` 目录下每个 `.md` 文件必须包含版本信息块和修改记录表：

### 文档头模板

```markdown
# 文档标题

> 文件：`文件名.md`
>
> 版本：v1.0
>
> 日期：YYYY-MM-DD
>
> 面向当前工程：（描述适用范围，可选）

# 修改记录

| 版本 | 日期 | 修改内容 |
|---|---|---|
| v1.1 | YYYY-MM-DD | 补充/修正/调整 说明（最新在最前） |
| v1.0 | YYYY-MM-DD | 创建文档：简要说明初始内容 |

---

（正文内容）
```

### 修改记录规则

- 每次实质性修改必须**在表格最前插入新行**（最新版本在最前）
- 版本号递增：`v1.0 → v1.1 → v1.2`；重大重构升主版本 `v1.x → v2.0`
- 文档头 `版本` 字段必须与修改记录表最新版本号保持一致
- 描述使用动词祈使句开头：如"新增…"、"补充…"、"修正…"

---

## Verilog 注释风格（source/verilog/）

### 文件头注释

```verilog
/**************************************************************************/
// Function   : 模块功能简述
// Version    : v1.0
// Date       : YYYY/MM/DD
// Description: 详细说明（多行）
//
// Modify:
// version       date       modify
// --------    -----------  ------------------------------------------------
//  v1.0        YYYY/MM/DD  创建文件
/**************************************************************************/
```

### 端口注释

在端口声明行末用 `//` 添加：

```verilog
input  wire                  clk             ,  // 系统时钟（100 MHz）
input  wire                  rstn            ,  // 低有效复位
input  wire [DATA_WIDTH-1:0] s_axis_tdata    ,  // AXI-Stream 数据输入
```

### 状态机注释

每个状态/状态转换条件须有中文说明：

```verilog
// 状态定义
localparam S_IDLE   = 2'd0;  // 空闲：等待 AXIS 数据
localparam S_FRAME  = 2'd1;  // 发送帧头（3 bit）
localparam S_DATA   = 2'd2;  // 发送数据（16 bit）
localparam S_PARITY = 2'd3;  // 发送奇偶校验

// 状态跳转说明写在 always 块上方
```
