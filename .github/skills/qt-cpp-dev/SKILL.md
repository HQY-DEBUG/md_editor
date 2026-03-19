---
name: qt-cpp-dev
description: "辅助本项目 Qt C++ Markdown 编辑器（mde/mde/）的开发，包括编辑器、预览、文件操作、信号槽、线程等。当涉及 mde/mde/ 目录下 C++ 文件、主窗口逻辑、Markdown 渲染、文件读写时使用。"
---

# Qt C++ Markdown 编辑器开发辅助（md_editor）

## 项目文件结构

| 文件 | 职责 |
|------|------|
| `main.cpp` | 程序入口，创建 `QApplication` 和主窗口 |
| `mde.h / mde.cpp` | 主窗口（`mde : QMainWindow`），编辑器核心逻辑 |
| `mde.ui` | Qt Designer UI 布局文件 |
| `mde.qrc` | 资源文件（图标、样式等） |
| `mde.rc` | Windows 应用图标资源 |

## 架构约定

### 线程模型

- **GUI 线程**：`mde`（QMainWindow），只做 UI 更新，不阻塞
- **工作线程**：`QThread` 子对象，负责文件 IO、Markdown 渲染等耗时操作
- 跨线程通信**只用 Qt 信号槽**（`Qt::QueuedConnection`），禁止从子线程直接操作控件

### 主窗口核心信号

```cpp
signals:
    void documentChanged(const QString &content);
    void fileSaved(const QString &path);
    void errorOccurred(const QString &msg);
```

### 槽命名约定

```cpp
private slots:
    void onDocumentChanged(const QString &content);
    void onFileSaved(const QString &path);
```

## 编辑器功能模块

### 文件操作

- 打开（`QFileDialog::getOpenFileName`）、保存（`QFile::write`）、另存为
- 修改状态跟踪：用 `m_is_modified` 标志，标题栏显示 `*` 提示未保存

### Markdown 预览

- 使用 `QWebEngineView` 或 `QTextBrowser` 渲染 HTML
- 编辑区内容变化时触发 `documentChanged` 信号，工作线程渲染 Markdown → HTML

### 快捷键

- 遵循常见编辑器习惯：`Ctrl+S` 保存，`Ctrl+O` 打开，`Ctrl+Z/Y` 撤销/重做

## 资源管理

- 优先使用 Qt 父子对象树管理生命周期，减少手动 `delete`
- `QThread` 工作对象在线程启动前完成连接，停止时调用 `quit()` + `wait()`

## 命名补充（Qt 部分）

| 类型 | 风格 | 示例 |
|------|------|------|
| 成员变量 | `m_` 前缀 | `m_editor`、`m_preview` |
| 信号名 | 动词过去式 | `documentChanged`、`fileSaved` |
| 槽名 | `on` + 信号名 | `onDocumentChanged` |
| 布尔成员 | `m_is_` / `m_has_` | `m_is_modified`、`m_has_selection` |


## 项目文件结构

| 文件 | 职责 |
|------|------|
| `main.cpp` | 程序入口，创建 `QApplication` 和主窗口 |
| `mainwindow.h/cpp` | 主窗口（`MainWindow`），所有 UI 逻辑（10 个功能分区） |
| `udpworker.h/cpp` | UDP 工作对象（`UdpWorker : QObject`），Winsock 原生 socket |
| `proto.h/cpp` | 协议编解码，所有 `CMD_*` 常量定义在 `Proto` 命名空间 |
| `xy2.h/cpp` | 备用/测试入口窗口 |
| `xy2.ui` | Qt Designer UI 文件 |

## 架构约定

### 线程模型

- **GUI 线程**：`MainWindow`，只做 UI 更新，不阻塞
- **UDP 工作对象**：`UdpWorker : QObject`（非 QThread），通过 `moveToThread` 推送到专用 `QThread`
- 跨线程通信**只用 Qt 信号槽**，禁止从子线程直接操作控件

### UdpWorker 核心信号

```cpp
signals:
    void receivedBatch(QList<QByteArray> pkts); // 批量收到的数据包
    void connected();
    void disconnected();
    void errorOccurred(const QString &msg);
```

### Windows 原生 Winsock（高吞吐）

- 目标吞吐：**25 万帧/秒**
- 发送/接收均使用原生 `socket()` + `sendto()` / `select` + `recvfrom`
- 发送/接收 socket 缓冲各 **4 MB**（`SO_SNDBUF` / `SO_RCVBUF`）
- 非 Windows 分支保留 `QUdpSocket` 实现

```cpp
// 典型 Winsock 初始化
WSADATA wsaData;
WSAStartup(MAKEWORD(2, 2), &wsaData);
sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
int sz = 4 * 1024 * 1024;
setsockopt(sock_, SOL_SOCKET, SO_SNDBUF, (char*)&sz, sizeof(sz));
setsockopt(sock_, SOL_SOCKET, SO_RCVBUF, (char*)&sz, sizeof(sz));
```

## 协议帧格式（Proto 命名空间）

```
FrameHeader(4) | CmdNum(4) | CmdType(4) | DataSize(4) |
Data(N)        | Checksum32(4) | FrameTail(4)

FrameHeader  = 0xA5A5A5A5
FrameTail    = 0xB5B5B5B5
Checksum32   = CmdNum + CmdType + DataSize + Σ Data[i]  (mod 2^32)
全部小端字节序
```

### 新增命令

1. 在 `proto.h` 中的对应功能区（`BB` 字节分类）添加 `constexpr quint32 CMD_XXX`
2. 在 `proto.cpp` 中实现对应的 `build_*` / `parse_*` 函数
3. 在 `mainwindow.cpp` 中登记 `PendingEntry` 并连接槽处理应答

## ACK 超时处理

```cpp
struct PendingEntry {
    qint64   sentAt;    // QDateTime::currentMSecsSinceEpoch()
    QString  label;     // 命令描述（用于日志）
    quint32  cmdType;
    bool     silentAck; // 批量操作静默 ACK
};
QHash<quint32, PendingEntry> m_pendingCmd; // key: CmdNum
```

- 应答到来后从 `m_pendingCmd` 移除
- 定时器周期扫描超时条目，首次超时告警 + 二次宽限机制

## 主窗口功能分区（mainwindow.h）

1. UDP 连接
2. 系统控制（版本读取、复位、网络配置下发）
3. 直接控制（GOTO XY、激光开关）
4. 扫描参数（Jump/Mark 速度、Jump 模式）
5. 激光参数（模式、脉冲、输出类型/引脚）
6. 列表状态与控制
7. 文件下发（校正/程序/列表/Jump 文件）
8. 协议测试（单次全协议/随机测试）
9. List 测试（5 种场景 + 长时间测试）
10. 日志区域

## 代码风格规范

- 缩进：4 个空格，K&R 大括号风格
- 命名：UI 成员指针用 `m_` 前缀；信号用动词过去式；槽用 `on` + 信号名
- 注释：全中文，函数头使用 Doxygen `@file/@brief/@param/@return`
- 文件头：`@file`、`@brief`、`@version`、`@date` + 修改记录

### 文件头模板

```cpp
/**
 * @file   xxx.cpp
 * @brief  模块简要说明
 * @version v1.0
 * @date    YYYY-MM-DD
 *
 * 修改记录（最新版本在最前）:
 *   vX.Y  YYYY-MM-DD  新增/修复/调整 说明
 *   v1.0  YYYY-MM-DD  创建文件
 */
```

## 与 Python 版对应关系

| Qt C++ 文件 | 对应 Python 文件 | 版本对齐 |
|------------|----------------|---------|
| `mainwindow.cpp` | `mainwindow.py` | `mainwindow.py v5.18` |
| `udpworker.cpp` | `udp_worker.py` | — |
| `proto.cpp` | `proto.py` | `proto.py v2.3` |
