/*
 * 文件 : mde.h
 * 描述 : MDE 主窗口类定义
 * 版本 : v1.1
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.1   dev      26/03/19   添加完整 UI 实现支持，引入工作线程架构
 * 1.0   dev      26/03/14   创建文件
 */
#pragma once

#include <QtWidgets/QMainWindow>
#include <QThread>
#include <QTimer>
#include "ui_mde.h"

/* 前向声明，减少头文件依赖 */
class MarkdownHighlighter;
class RenderWorker;
class FileWorker;

QT_BEGIN_NAMESPACE
namespace Ui { class mdeClass; }
QT_END_NAMESPACE

/**
 * @brief MDE 主窗口（协调器）
 *
 * 负责：
 *  - 持有并管理各工作线程（渲染线程、文件 IO 线程）
 *  - 连接菜单/工具栏动作与具体操作
 *  - 协调编辑器、预览、侧边栏之间的数据流
 *
 * GUI 线程只做 UI 更新，耗时操作均委托给工作线程（Qt::QueuedConnection）。
 */
class mde : public QMainWindow {
    Q_OBJECT

public:
    explicit mde(QWidget *parent = nullptr);
    ~mde();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // --- 文件菜单槽 ---//
    void onActionNew();
    void onActionOpen();
    void onActionSave();
    void onActionSaveAs();
    void onActionClose();
    void onActionExit();
    void onActionExportHTML();
    void onActionExportPDF();

    // --- 编辑菜单槽（Find/Replace 需自定义） ---//
    void onActionFind();
    void onActionReplace();

    // --- 格式菜单槽 ---//
    void onActionBold();
    void onActionItalic();
    void onActionStrikethrough();
    void onActionInlineCode();
    void onActionHeading1();
    void onActionHeading2();
    void onActionHeading3();
    void onActionHeading4();
    void onActionHeading5();
    void onActionHeading6();
    void onActionBulletList();
    void onActionOrderedList();
    void onActionTaskList();
    void onActionBlockquote();
    void onActionCodeBlock();
    void onActionInsertLink();
    void onActionInsertImage();
    void onActionInsertTable();
    void onActionHorizontalRule();

    // --- 视图菜单槽 ---//
    void onActionToggleSidebar(bool checked);
    void onActionTogglePreview(bool checked);
    void onActionFocusMode(bool checked);
    void onActionFullscreen(bool checked);
    void onActionZoomIn();
    void onActionZoomOut();
    void onActionZoomReset();

    // --- 帮助菜单槽 ---//
    void onActionShortcuts();
    void onActionAbout();

    // --- 内部协调槽 ---//
    void onEditorTextChanged();           ///< 编辑器内容变化，重置防抖定时器
    void onRenderDebounceTimeout();       ///< 防抖到期，触发渲染和大纲更新
    void onCursorPositionChanged();       ///< 游标移动，更新状态栏行列信息

    void onRenderFinished(const QString &html);              ///< 工作线程渲染完成
    void onFileLoaded(const QString &content,
                      const QString &path);                  ///< 工作线程加载完成
    void onFileSaved(const QString &path);                   ///< 工作线程保存完成
    void onExportFinished(const QString &path);              ///< 工作线程导出完成
    void onWorkerError(const QString &msg);                  ///< 工作线程报错

    void onOutlineItemClicked(QTreeWidgetItem *item, int column); ///< 大纲跳转

signals:
    /* 向工作线程发送任务（均使用 Qt::QueuedConnection） */
    void requestRender(const QString &markdown);
    void requestLoadFile(const QString &path);
    void requestSaveFile(const QString &path, const QString &content);
    void requestExportHTML(const QString &path, const QString &html);

private:
    // --- 初始化方法 ---//
    void setup_workers();      ///< 创建工作线程并完成信号槽连接
    void setup_connections();  ///< 连接菜单 action 到对应槽
    void setup_editor();       ///< 编辑器初始设置（高亮、Tab 宽度等）
    void setup_icons();        ///< 为所有 action 和工具栏按钮设置图标

    // --- 编辑器格式化辅助 ---//
    /**
     * @brief 在选区或光标处包裹 Markdown 语法标记
     * @param prefix 前缀符号（如 **）
     * @param suffix 后缀符号（如 **）
     */
    void insert_wrap(const QString &prefix, const QString &suffix);

    /**
     * @brief 在当前行首插入行前缀
     * @param prefix 前缀字符串（如 "- "、"> "）
     */
    void insert_line_prefix(const QString &prefix);

    /**
     * @brief 将当前行设为指定级别的标题
     * @param level 标题级别（1–6）
     */
    void apply_heading(int level);

    // --- 状态管理辅助 ---//
    void    update_title();          ///< 根据文件名和修改状态刷新窗口标题
    void    update_outline();        ///< 解析当前内容并刷新大纲树
    void    update_status_bar();     ///< 刷新状态栏行/列信息
    void    set_modified(bool val);  ///< 设置修改状态并刷新标题
    bool    confirm_save();          ///< 若有未保存内容，弹窗询问是否保存；返回是否可以继续
    void    open_file(const QString &path);
    void    set_editor_content(const QString &content); ///< 静默加载内容（不触发 modified）

    // --- 成员变量 ---//
    Ui::mdeClass        *ui;

    MarkdownHighlighter *m_highlighter;    ///< 编辑器语法高亮（由 QTextDocument 持有）
    RenderWorker        *m_render_worker;  ///< 渲染工作对象
    QThread             *m_render_thread; ///< 渲染工作线程
    FileWorker          *m_file_worker;   ///< 文件 IO 工作对象
    QThread             *m_file_thread;   ///< 文件 IO 工作线程

    QTimer              *m_render_timer;  ///< 防抖定时器（300 ms）

    QString              m_current_file;  ///< 当前打开的文件路径，空 = 未命名
    bool                 m_is_modified;   ///< 是否有未保存的修改
    int                  m_base_font_size;///< 编辑器初始字号（用于缩放还原）
};
