---
description: "Git 提交消息规范：中文 Conventional Commits 格式。当生成或编写 git commit message 时使用。"
applyTo: "**/*"
---

# Git 提交消息规范

## 格式

```
<类型>(<范围>): <简短中文描述>

<可选：详细说明>
```

## 类型

| 类型 | 用途 |
|------|------|
| `feat` | 新功能 |
| `fix` | Bug 修复 |
| `refactor` | 重构（不影响功能） |
| `docs` | 文档更新 |
| `style` | 代码格式调整 |
| `test` | 测试相关 |
| `chore` | 构建/工具/依赖变更 |

## 范围（本项目常用）

`editor`、`preview`、`ui`、`file`、`doc`、`build`

## 示例

```
feat(editor): 新增语法高亮支持
```

```
fix(preview): 修复代码块渲染换行异常
```

```
docs(doc): 更新快捷键说明文档
```

## 注意事项

- 简短描述不超过 50 字符（中文约 25 字）
- 使用祈使句（"新增"而非"新增了"）
- 提交前确认 `doc/版本记录.md` 是否需要同步更新
