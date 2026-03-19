---
description: "Qt C++ 特定规范：信号槽声明、线程安全、QObject 继承约定。当编写或修改 mde/ 目录下 .cpp/.h 文件时使用。"
applyTo: "mde/**/*.cpp,mde/**/*.h"
---

# Qt C++ 代码规范

> 本规则是 `c-cpp-style` 的补充，适用于 `mde/` 目录下的 Qt 代码。

## 线程安全

- **GUI 线程**（`QWidget` / `QMainWindow`）只做 UI 更新，不阻塞
- **工作线程**（`QThread` 子类）负责网络、IO 等耗时操作
- 跨线程通信**只用 Qt 信号槽**（`Qt::QueuedConnection`），禁止从子线程直接操作控件

## 信号槽声明

```cpp
// 信号声明在 signals: 区域
signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &msg);

// 槽声明在 public slots: 或 private slots: 区域
private slots:
    void onDataReceived(const QByteArray &data);
```

## QObject 继承

- 继承 `QObject` 的类必须在第一行包含 `Q_OBJECT` 宏
- 构造函数接受 `QWidget *parent = nullptr` 参数并传给基类

```cpp
class UdpWorker : public QThread {
    Q_OBJECT
public:
    explicit UdpWorker(QObject *parent = nullptr);
    ...
};
```

## 资源管理

- 优先使用 Qt 父子对象树管理生命周期，减少手动 `delete`
- `QThread` 工作对象在线程启动前完成连接，停止时调用 `quit()` + `wait()`

## 命名补充

- 信号名：动词过去式或名词，如 `dataReceived`、`connected`
- 槽名：`on` + 信号名，如 `onDataReceived`
- UI 成员指针：`m_` 前缀，如 `m_btnSend`
