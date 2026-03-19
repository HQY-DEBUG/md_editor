---
description: "变量与标识符命名规范：C/C++。当编写或修改 .c/.h/.cpp/.hpp 文件时使用。"
applyTo: "**/*.c,**/*.h,**/*.cpp,**/*.hpp"
---

# 命名规范

## C/C++

| 类型 | 风格 | 示例 |
|------|------|------|
| 变量、函数 | 小写下划线 | `doc_content`、`load_file()` |
| 宏定义 | 全大写下划线 | `MAX_FILE_SIZE` |
| 全局变量 | `g_` 前缀 + 小写下划线 | `g_app_config` |
| 类名 | 大驼峰 | `MarkdownEditor` |
| 成员变量 | `m_` 前缀 + 小写下划线 | `m_editor`、`m_preview` |
| 信号名 | 动词过去式或名词 | `documentChanged`、`fileSaved` |
| 槽名 | `on` + 信号名（大驼峰） | `onDocumentChanged` |

## 通用原则

- 命名要有意义，避免单字母（除循环变量 `i`、`j`、`k`）
- 布尔变量用 `is_`、`has_`、`can_` 等前缀，如 `is_modified`、`has_selection`
