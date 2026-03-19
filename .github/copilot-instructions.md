# md_editor 项目工作区指引

## 语言要求

**所有回答、解释、思考过程、代码注释建议均使用中文。**

## 项目概览

md_editor 是一款基于 Qt C++ 的 Markdown 编辑器（Visual Studio + Qt 方案）。

| 目录 | 语言/框架 | 用途 |
|------|-----------|------|
| `doc/` | Markdown | 项目文档、功能说明、接口说明 |
| `mde/mde/` | C++ / Qt | 编辑器主程序源码（QMainWindow） |

## 项目结构

```
md_editor/
├── doc/                    # 说明文档（Markdown 格式）
├── mde/                    # Qt 源代码目录（Visual Studio 解决方案）
│   ├── mde/                # Qt C++ 项目源码
│   │   ├── main.cpp        # 程序入口
│   │   ├── mde.cpp / .h    # 主窗口实现（QMainWindow）
│   │   ├── mde.ui          # Qt UI 定义文件
│   │   ├── mde.qrc         # Qt 资源文件
│   │   ├── mde.rc          # 应用图标资源
│   │   └── mde.vcxproj     # Visual Studio 项目文件
│   └── mde.slnx            # Visual Studio 解决方案文件
├── README.md
└── LICENSE
```

## 版本控制规范

- 修改已有函数或接口时，说明此改动对其他模块的影响
- 不随意删除现有代码；废弃代码用注释标注原因，保留一段时间后再清理
- 新增功能在注释中标注修改日期，例如：
  ```cpp
  /* 2026-03-19 新增：增加语法高亮功能 */
  ```
- 重大结构性改动前提醒用户先 `git commit` 保存当前进度
- `doc/版本记录.md` 需同步更新当前变更说明

## Git 提交消息规范

格式：`<类型>(<范围>): <简短中文描述>`

| 类型 | 用途 |
|------|------|
| `feat` | 新功能 |
| `fix` | Bug 修复 |
| `refactor` | 重构 |
| `docs` | 文档更新 |
| `style` | 代码格式 |
| `chore` | 构建/工具变更 |

常用范围：`editor`、`preview`、`ui`、`file`、`doc`、`build`

示例：`feat(editor): 新增语法高亮支持`

- 描述不超过 50 字符，使用祈使句（"新增"而非"新增了"）

## 架构要点

### Qt C++ 主程序（`mde/mde/`）
- **GUI 线程**（`QMainWindow`）：只做 UI 更新，不阻塞
- **工作线程**（`QThread`）：负责文件 IO、渲染等耗时操作
- 跨线程通信**只用 Qt 信号槽**（`Qt::QueuedConnection`），禁止从子线程直接操作控件
- 优先使用 Qt 父子对象树管理生命周期，减少手动 `delete`
- `QThread` 工作对象在线程启动前完成连接，停止时调用 `quit()` + `wait()`

## 代码风格速查

| 语言 | 缩进 | 括号风格 |
|------|------|----------|
| C++ | 4 空格 | K&R（左括号不换行） |

## 可用 Skills（`.github/skills/`）

| Skill | 触发场景 |
|-------|---------|
| `qt-cpp-dev` | `mde/mde/` 下 Qt C++ 代码、Markdown 编辑器开发 |
| `doc-generation` | 为代码生成中文注释/文档 |
| `git-commit-msg` | 生成规范的中文 Git 提交消息 |

详细规范见 `.github/instructions/` 下对应文件：

- `c-cpp-style.instructions.md` — C/C++ 缩进、括号、文件头、Doxygen 注释
- `qt-cpp-style.instructions.md` — Qt C++ 信号槽、线程安全、QObject 约定
- `naming-conventions.instructions.md` — C/C++ 变量/函数/宏命名规范
- `git-commit.instructions.md` — Git 提交消息格式（中文 Conventional Commits）
- `doc-style.instructions.md` — `doc/` 目录 Markdown 文档版本控制格式

## 文档规范

- 新增文档放入 `doc/` 目录，使用 `.md` 格式
- `doc/` 下每个 `.md` 文件**必须包含版本控制头**，格式如下：
  ```markdown
  # 文档标题

  > 文件：`文件名.md`
  >
  > 版本：v1.0
  >
  > 日期：YYYY-MM-DD

  # 修改记录

  | 版本 | 日期 | 修改内容 |
  |---|---|---|
  | v1.0 | YYYY-MM-DD | 创建文档：说明 |
  （最新版本排在**最前面**，倒序）
  ```
- 每次实质性修改须在修改记录表中追加一行，版本号与文档头保持一致
- 详细格式规范见 `doc-style.instructions.md`
- 代码注释使用中文
