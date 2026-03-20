# 源文件说明

本文件介绍 MDE 项目中每个源代码文件的职责、所属层次及对外接口。

---

## 目录结构

```
mde/mde/
├── main.cpp
├── mde.h / mde.cpp
├── markdown_highlighter.h / markdown_highlighter.cpp
├── code_highlighter.h / code_highlighter.cpp
├── render_worker.h / render_worker.cpp
├── file_worker.h / file_worker.cpp
└── outline_parser.h / outline_parser.cpp
```

---

## 架构分层

```
┌─────────────────────────────────────────────┐
│               GUI 线程                       │
│  mde（主窗口）                               │
│  MarkdownHighlighter  OutlineParser          │
└───────────┬──────────────────────────────────┘
            │ Qt::QueuedConnection 信号槽
┌───────────▼──────────────────────────────────┐
│               工作线程                        │
│  RenderWorker（渲染线程）                     │
│  FileWorker  （文件 IO 线程）                 │
└─────────────────────────────────────────────┘
```

GUI 线程只做 UI 更新，所有耗时操作委托给对应工作线程。跨线程通信仅使用 `Qt::QueuedConnection`，禁止从子线程直接操作控件。

---

## 文件说明

### `main.cpp`

**职责**：程序入口。

创建 `QApplication` 和主窗口 `mde`，启动事件循环。无业务逻辑。

---

### `mde.h` / `mde.cpp`

**职责**：主窗口（协调器）。

| 类 | 基类 |
|----|------|
| `mde` | `QMainWindow` |

承担以下责任：
- 持有并管理渲染工作线程（`RenderWorker` + `QThread`）和文件 IO 工作线程（`FileWorker` + `QThread`）
- 连接菜单 / 工具栏动作到对应槽
- 协调编辑器、预览区、侧边栏大纲之间的数据流
- 维护应用状态：当前文件路径（`m_current_file`）、是否已修改（`m_is_modified`）

**对外信号**（向工作线程派发任务）：

| 信号 | 接收方 | 说明 |
|------|--------|------|
| `requestRender(markdown)` | `RenderWorker::render` | 触发 Markdown → HTML 渲染 |
| `requestLoadFile(path)` | `FileWorker::load_file` | 读取文件 |
| `requestSaveFile(path, content)` | `FileWorker::save_file` | 保存文件 |
| `requestExportHTML(path, html)` | `FileWorker::export_html` | 导出 HTML |

**关键私有方法**：

| 方法 | 说明 |
|------|------|
| `setup_workers()` | 创建工作线程，完成信号槽连接 |
| `setup_connections()` | 绑定菜单 / 工具栏 action 到槽 |
| `setup_icons()` | 数据驱动方式为所有 action 设置 SVG 图标 |
| `set_editor_content(content)` | 静默加载内容（`blockSignals` 屏蔽 modified 标记） |
| `clear_document()` | 清空编辑器、重置文件状态 |
| `update_outline(content)` | 调用 `OutlineParser` 刷新侧边栏大纲树 |
| `confirm_save()` | 有未保存内容时弹窗询问，返回是否可继续 |
| `insert_wrap(prefix, suffix)` | 在选区两端插入 Markdown 标记（粗体、斜体等） |
| `apply_heading(level)` | 将当前行替换为指定级别的标题 |

---

### `markdown_highlighter.h` / `markdown_highlighter.cpp`

**职责**：Markdown 编辑器语法高亮。

| 类 | 基类 |
|----|------|
| `MarkdownHighlighter` | `QSyntaxHighlighter` |
| `CodeBlockData` | `QTextBlockUserData` |

挂载到 `QPlainTextEdit` 的 `QTextDocument`，由框架逐块调用 `highlightBlock()`。

**高亮内容**：标题（H1–H6）、粗体、斜体、删除线、行内代码、链接 / 图片、引用块、无序 / 有序 / 任务列表、分隔线、多行代码块。

**多行代码块状态机**：

| 块状态值 | 含义 |
|---------|------|
| `0` | 普通 Markdown 文本 |
| `1` | 处于代码块（` ``` `）内部 |

检测到 `` ```lang `` 围栏行时，将语言名规范化后存入当前块的 `CodeBlockData`；代码块内每行都从前一块的 `CodeBlockData` 继承语言，并委托 `CodeHighlighter::rules_for()` 进行语言语法着色（叠加在代码块基础背景格式之上）。

---

### `code_highlighter.h` / `code_highlighter.cpp`

**职责**：代码块语言语法高亮规则库。

| 类 | 类型 |
|----|------|
| `CodeHighlighter` | 纯静态工具类（禁止实例化） |
| `CodeRule` | 数据结构（`pattern` + `format`） |

对外接口：

| 方法 | 说明 |
|------|------|
| `rules_for(lang)` | 返回指定语言的规则列表（静态缓存，每种语言只构建一次） |
| `normalize(raw)` | 将别名映射到标准语言名（如 `"c++"` → `"cpp"`） |

**支持的语言**：

| 标准名 | 支持的围栏标识符 | 高亮特性 |
|--------|----------------|---------|
| `cpp` | `c`, `cpp`, `c++`, `cxx`, `cc`, `h`, `hpp`, `hxx` | 关键字、预处理指令、字符串、数值、注释 |
| `python` | `python`, `py` | 关键字、内置函数、装饰器、字符串、数值、注释 |
| `js` | `javascript`, `js`, `jsx`, `typescript`, `ts`, `tsx` | 关键字（含 TS 扩展）、模板字符串、字符串、数值、注释 |
| `java` | `java` | 关键字、注解、字符串、数值、注释 |
| `bash` | `bash`, `sh`, `shell`, `zsh`, `fish` | 关键字、内置命令、变量引用（`$VAR`）、字符串、注释 |
| `go` | `go`, `golang` | 关键字、内置函数、原始字符串（反引号）、字符串、数值、注释 |
| `rust` | `rust`, `rs` | 关键字、基础类型、宏调用、生命周期注解、字符串、数值、注释 |
| `sql` | `sql` | 关键字（大小写不敏感）、字符串、数值、注释 |
| `json` | `json` | 键名（紫色）、字符串值（绿色）、数值、布尔 / null |
| `verilog` | `verilog`, `v`, `sv`, `systemverilog` | 关键字（含 SystemVerilog 扩展）、编译器指令（`` ` ``）、系统任务（`$`）、位宽数值（`4'b1010`）、字符串、注释 |

规则按优先级从低到高排列，后应用的规则会覆盖先应用的格式（注释始终最后应用，优先级最高）。

---

### `render_worker.h` / `render_worker.cpp`

**职责**：在独立工作线程中将 Markdown 渲染为 HTML。

| 类 | 基类 |
|----|------|
| `RenderWorker` | `QObject` |

通过 `QObject::moveToThread()` 运行于独立 `QThread`，不继承 `QThread`。

**槽（由 GUI 线程通过 QueuedConnection 调用）**：

| 槽 | 说明 |
|----|------|
| `render(markdown)` | 在栈上创建局部 `QTextDocument`，调用 `setMarkdown()`，emit `renderFinished(html)` |

**信号**：

| 信号 | 说明 |
|------|------|
| `renderFinished(html)` | 渲染完成，返回 HTML 字符串给 GUI 线程 |
| `errorOccurred(msg)` | 渲染出错（当前实现不触发） |

使用 Qt 5.14+ 内置的 GitHub Flavored Markdown 解析器（`QTextDocument::MarkdownDialectGitHub`）。`QTextDocument` 在工作线程栈上局部创建，不跨线程共享，线程安全。

---

### `file_worker.h` / `file_worker.cpp`

**职责**：在独立工作线程中处理文件读写，避免大文件 IO 阻塞 GUI 线程。

| 类 | 基类 |
|----|------|
| `FileWorker` | `QObject` |

**槽（公开，由 GUI 线程调用）**：

| 槽 | 说明 |
|----|------|
| `load_file(path)` | 以 UTF-8 读取文件，emit `fileLoaded(content, path)` |
| `save_file(path, content)` | 以 UTF-8 写入文件，emit `fileSaved(path)` |
| `export_html(path, html_content)` | 以 UTF-8 写入 HTML，emit `exportFinished(path)` |

**信号**：

| 信号 | 说明 |
|------|------|
| `fileLoaded(content, path)` | 文件读取成功 |
| `fileSaved(path)` | 文件保存成功 |
| `exportFinished(path)` | HTML 导出成功 |
| `errorOccurred(msg)` | 文件操作失败 |

**私有方法**：

| 方法 | 说明 |
|------|------|
| `write_text_file(path, content, error_prefix)` | `save_file` / `export_html` 共用的写文件辅助，返回是否成功 |

文件对象析构时自动关闭（RAII），无需手动调用 `close()`。

---

### `outline_parser.h` / `outline_parser.cpp`

**职责**：从 Markdown 文本中提取标题，构建大纲列表。

| 类 / 结构 | 类型 |
|----------|------|
| `OutlineParser` | 纯静态工具类（禁止实例化） |
| `OutlineItem` | 数据结构（`level`、`title`、`line_number`） |

速度足够快，直接在 GUI 线程调用，无需工作线程。

**接口**：

```cpp
static QList<OutlineItem> parse(const QString &markdown);
```

解析规则：
- 仅识别 ATX 格式标题（行首 `#` ~ `######`）
- 跳过代码块（` ``` ` 之间）内部的 `#` 行，避免误判
- 返回按行号升序排列的 `OutlineItem` 列表，由 `mde::update_outline()` 构建为层级树

---

## 依赖关系

```
main.cpp
  └─ mde
       ├─ MarkdownHighlighter
       │    └─ CodeHighlighter
       ├─ RenderWorker
       ├─ FileWorker
       └─ OutlineParser
```

`CodeHighlighter`、`OutlineParser` 为纯工具类，无 Qt 对象树依赖。其余类均挂载于 Qt 父子对象树中，生命周期由框架管理。
