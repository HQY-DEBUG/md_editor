/*
 * 文件 : mde.cpp
 * 描述 : MDE 主窗口实现
 * 版本 : v1.2
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.3   dev      26/03/20   新增主题切换功能（亮色/暗色/护眼/高对比度），注入预览 CSS
 * 1.2   dev      26/03/19   优化：提取 clear_document、update_outline 参数化避免二次读取、blockSignals 替换 disconnect/connect、图标设置数据驱动
 * 1.1   dev      26/03/19   实现完整 UI 交互逻辑及工作线程协调
 * 1.0   dev      26/03/14   创建文件
 */
#include "mde.h"
#include "markdown_highlighter.h"
#include "render_worker.h"
#include "file_worker.h"
#include "outline_parser.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QCloseEvent>
#include <QTextCursor>
#include <QTextDocument>
#include <QTreeWidgetItem>
#include <QFileInfo>
#include <QScrollBar>
#include <QStatusBar>
#include <QRegularExpression>

// ============================================================
//  构造 / 析构
// ============================================================

mde::mde(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mdeClass())
    , m_highlighter(nullptr)
    , m_render_worker(nullptr)
    , m_render_thread(nullptr)
    , m_file_worker(nullptr)
    , m_file_thread(nullptr)
    , m_render_timer(nullptr)
    , m_theme_group(nullptr)
    , m_is_modified(false)
    , m_base_font_size(12)
{
    ui->setupUi(this);

    setup_workers();
    setup_editor();
    setup_connections();
    setup_icons();
    setup_theme_group();   /* 2026-03-20 新增：加载并应用保存的主题 */

    update_title();
    statusBar()->showMessage(QStringLiteral("就绪"));
}

mde::~mde()
{
    /* 停止工作线程（quit 发送退出请求，wait 等待线程实际结束） */
    if (m_render_thread) {
        m_render_thread->quit();
        m_render_thread->wait();
    }
    if (m_file_thread) {
        m_file_thread->quit();
        m_file_thread->wait();
    }

    delete ui;
}

// ============================================================
//  初始化
// ============================================================

void mde::setup_workers()
{
    // --- 渲染工作线程 ---//
    m_render_worker = new RenderWorker();        /* 无 parent，稍后 moveToThread */
    m_render_thread = new QThread(this);
    m_render_worker->moveToThread(m_render_thread);

    connect(this,            &mde::requestRender,
            m_render_worker, &RenderWorker::render,
            Qt::QueuedConnection);
    connect(m_render_worker, &RenderWorker::renderFinished,
            this,            &mde::onRenderFinished,
            Qt::QueuedConnection);
    connect(m_render_worker, &RenderWorker::errorOccurred,
            this,            &mde::onWorkerError,
            Qt::QueuedConnection);
    connect(m_render_thread, &QThread::finished,
            m_render_worker, &QObject::deleteLater);

    m_render_thread->start();

    // --- 文件 IO 工作线程 ---//
    m_file_worker = new FileWorker();
    m_file_thread = new QThread(this);
    m_file_worker->moveToThread(m_file_thread);

    connect(this,          &mde::requestLoadFile,
            m_file_worker, &FileWorker::load_file,
            Qt::QueuedConnection);
    connect(this,          &mde::requestSaveFile,
            m_file_worker, &FileWorker::save_file,
            Qt::QueuedConnection);
    connect(this,          &mde::requestExportHTML,
            m_file_worker, &FileWorker::export_html,
            Qt::QueuedConnection);
    connect(m_file_worker, &FileWorker::fileLoaded,
            this,          &mde::onFileLoaded,
            Qt::QueuedConnection);
    connect(m_file_worker, &FileWorker::fileSaved,
            this,          &mde::onFileSaved,
            Qt::QueuedConnection);
    connect(m_file_worker, &FileWorker::exportFinished,
            this,          &mde::onExportFinished,
            Qt::QueuedConnection);
    connect(m_file_worker, &FileWorker::errorOccurred,
            this,          &mde::onWorkerError,
            Qt::QueuedConnection);
    connect(m_file_thread, &QThread::finished,
            m_file_worker, &QObject::deleteLater);

    m_file_thread->start();

    // --- 防抖定时器（300 ms 无输入后触发渲染） ---//
    m_render_timer = new QTimer(this);
    m_render_timer->setSingleShot(true);
    m_render_timer->setInterval(300);
    connect(m_render_timer, &QTimer::timeout,
            this,           &mde::onRenderDebounceTimeout);
}

void mde::setup_editor()
{
    /* 隐藏侧边栏 Dock 的标题条（固定高度 0 彻底消除占位） */
    auto *empty_title = new QWidget(this);
    empty_title->setFixedHeight(0);
    ui->sidebarDock->setTitleBarWidget(empty_title);

    /* 挂载语法高亮器到编辑器的 QTextDocument */
    m_highlighter = new MarkdownHighlighter(ui->editorWidget->document());

    /* Tab 宽度设为 4 个空格的视觉宽度 */
    ui->editorWidget->setTabStopDistance(
        QFontMetricsF(ui->editorWidget->font()).horizontalAdvance(QLatin1Char(' ')) * 4);
}

void mde::setup_connections()
{
    // --- 文件菜单 ---//
    connect(ui->actionNew,         &QAction::triggered, this, &mde::onActionNew);
    connect(ui->actionOpen,        &QAction::triggered, this, &mde::onActionOpen);
    connect(ui->actionSave,        &QAction::triggered, this, &mde::onActionSave);
    connect(ui->actionSaveAs,      &QAction::triggered, this, &mde::onActionSaveAs);
    connect(ui->actionClose,       &QAction::triggered, this, &mde::onActionClose);
    connect(ui->actionExit,        &QAction::triggered, this, &mde::onActionExit);
    connect(ui->actionExportHTML,  &QAction::triggered, this, &mde::onActionExportHTML);
    connect(ui->actionExportPDF,   &QAction::triggered, this, &mde::onActionExportPDF);

    // --- 编辑菜单（Undo/Redo/Cut/Copy/Paste/SelectAll 委托给控件内置操作） ---//
    connect(ui->actionUndo,      &QAction::triggered, ui->editorWidget, &QPlainTextEdit::undo);
    connect(ui->actionRedo,      &QAction::triggered, ui->editorWidget, &QPlainTextEdit::redo);
    connect(ui->actionCut,       &QAction::triggered, ui->editorWidget, &QPlainTextEdit::cut);
    connect(ui->actionCopy,      &QAction::triggered, ui->editorWidget, &QPlainTextEdit::copy);
    connect(ui->actionPaste,     &QAction::triggered, ui->editorWidget, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, ui->editorWidget, &QPlainTextEdit::selectAll);
    connect(ui->actionFind,      &QAction::triggered, this, &mde::onActionFind);
    connect(ui->actionReplace,   &QAction::triggered, this, &mde::onActionReplace);

    // --- 格式菜单 ---//
    connect(ui->actionBold,          &QAction::triggered, this, &mde::onActionBold);
    connect(ui->actionItalic,        &QAction::triggered, this, &mde::onActionItalic);
    connect(ui->actionStrikethrough, &QAction::triggered, this, &mde::onActionStrikethrough);
    connect(ui->actionInlineCode,    &QAction::triggered, this, &mde::onActionInlineCode);
    connect(ui->actionHeading1,      &QAction::triggered, this, &mde::onActionHeading1);
    connect(ui->actionHeading2,      &QAction::triggered, this, &mde::onActionHeading2);
    connect(ui->actionHeading3,      &QAction::triggered, this, &mde::onActionHeading3);
    connect(ui->actionHeading4,      &QAction::triggered, this, &mde::onActionHeading4);
    connect(ui->actionHeading5,      &QAction::triggered, this, &mde::onActionHeading5);
    connect(ui->actionHeading6,      &QAction::triggered, this, &mde::onActionHeading6);
    connect(ui->actionBulletList,    &QAction::triggered, this, &mde::onActionBulletList);
    connect(ui->actionOrderedList,   &QAction::triggered, this, &mde::onActionOrderedList);
    connect(ui->actionTaskList,      &QAction::triggered, this, &mde::onActionTaskList);
    connect(ui->actionBlockquote,    &QAction::triggered, this, &mde::onActionBlockquote);
    connect(ui->actionCodeBlock,     &QAction::triggered, this, &mde::onActionCodeBlock);
    connect(ui->actionInsertLink,    &QAction::triggered, this, &mde::onActionInsertLink);
    connect(ui->actionInsertImage,   &QAction::triggered, this, &mde::onActionInsertImage);
    connect(ui->actionInsertTable,   &QAction::triggered, this, &mde::onActionInsertTable);
    connect(ui->actionHorizontalRule,&QAction::triggered, this, &mde::onActionHorizontalRule);

    // --- 视图菜单 ---//
    connect(ui->actionToggleSidebar, &QAction::toggled,  this, &mde::onActionToggleSidebar);
    connect(ui->actionTogglePreview, &QAction::toggled,  this, &mde::onActionTogglePreview);
    connect(ui->actionFocusMode,     &QAction::toggled,  this, &mde::onActionFocusMode);
    connect(ui->actionFullscreen,    &QAction::toggled,  this, &mde::onActionFullscreen);
    connect(ui->actionZoomIn,        &QAction::triggered, this, &mde::onActionZoomIn);
    connect(ui->actionZoomOut,       &QAction::triggered, this, &mde::onActionZoomOut);
    connect(ui->actionZoomReset,     &QAction::triggered, this, &mde::onActionZoomReset);

    // --- 帮助菜单 ---//
    connect(ui->actionShortcuts, &QAction::triggered, this, &mde::onActionShortcuts);
    connect(ui->actionAbout,     &QAction::triggered, this, &mde::onActionAbout);

    // --- 编辑器内容变化 ---//
    connect(ui->editorWidget, &QPlainTextEdit::textChanged,
            this,             &mde::onEditorTextChanged);
    connect(ui->editorWidget, &QPlainTextEdit::cursorPositionChanged,
            this,             &mde::onCursorPositionChanged);

    // --- 大纲点击跳转 ---//
    connect(ui->outlineTreeWidget, &QTreeWidget::itemClicked,
            this,                  &mde::onOutlineItemClicked);

    // --- 侧边栏 Dock 被用户手动关闭时同步 action 状态 ---//
    connect(ui->sidebarDock, &QDockWidget::visibilityChanged,
            ui->actionToggleSidebar, &QAction::setChecked);

    // --- 主题菜单 ---//
    connect(ui->actionThemeLight,        &QAction::triggered, this, &mde::onActionThemeTriggered);
    connect(ui->actionThemeDark,         &QAction::triggered, this, &mde::onActionThemeTriggered);
    connect(ui->actionThemeSepia,        &QAction::triggered, this, &mde::onActionThemeTriggered);
    connect(ui->actionThemeHighContrast, &QAction::triggered, this, &mde::onActionThemeTriggered);
}

// ============================================================
//  文件操作槽
// ============================================================

void mde::onActionNew()
{
    if (!confirm_save()) return;
    clear_document();
}

void mde::onActionOpen()
{
    if (!confirm_save()) return;

    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("打开 Markdown 文件"),
        QString(),
        QStringLiteral("Markdown 文件 (*.md *.markdown);;所有文件 (*)")
    );
    if (!path.isEmpty()) {
        emit requestLoadFile(path);
    }
}

void mde::onActionSave()
{
    if (m_current_file.isEmpty()) {
        onActionSaveAs();
    } else {
        emit requestSaveFile(m_current_file, ui->editorWidget->toPlainText());
    }
}

void mde::onActionSaveAs()
{
    const QString path = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("另存为"),
        QString(),
        QStringLiteral("Markdown 文件 (*.md *.markdown);;所有文件 (*)")
    );
    if (!path.isEmpty()) {
        emit requestSaveFile(path, ui->editorWidget->toPlainText());
    }
}

void mde::onActionClose()
{
    if (!confirm_save()) return;
    clear_document();
}

void mde::onActionExit()
{
    close();
}

void mde::onActionExportHTML()
{
    const QString path = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("导出为 HTML"),
        QString(),
        QStringLiteral("HTML 文件 (*.html *.htm)")
    );
    if (path.isEmpty()) return;

    /* 先触发同步渲染获取 HTML，再发送给文件线程写入 */
    QTextDocument doc;
    doc.setMarkdown(ui->editorWidget->toPlainText(),
                    QTextDocument::MarkdownDialectGitHub);
    emit requestExportHTML(path, doc.toHtml());
}

void mde::onActionExportPDF()
{
    QMessageBox::information(this,
        QStringLiteral("导出 PDF"),
        QStringLiteral("PDF 导出功能待实现。"));
}

// ============================================================
//  编辑操作槽
// ============================================================

void mde::onActionFind()
{
    bool ok = false;
    const QString keyword = QInputDialog::getText(
        this,
        QStringLiteral("查找"),
        QStringLiteral("查找内容："),
        QLineEdit::Normal, QString(), &ok
    );
    if (!ok || keyword.isEmpty()) return;

    /* 从当前位置向下查找；未找到则回到文档头重试 */
    if (!ui->editorWidget->find(keyword)) {
        QTextCursor cursor = ui->editorWidget->textCursor();
        cursor.movePosition(QTextCursor::Start);
        ui->editorWidget->setTextCursor(cursor);

        if (!ui->editorWidget->find(keyword)) {
            QMessageBox::information(this,
                QStringLiteral("查找"),
                QStringLiteral("未找到 \"%1\"").arg(keyword));
        }
    }
}

void mde::onActionReplace()
{
    QMessageBox::information(this,
        QStringLiteral("替换"),
        QStringLiteral("替换功能待实现。"));
}

// ============================================================
//  格式化操作槽
// ============================================================

void mde::onActionBold()          { insert_wrap(QStringLiteral("**"), QStringLiteral("**")); }
void mde::onActionItalic()        { insert_wrap(QStringLiteral("*"),  QStringLiteral("*"));  }
void mde::onActionStrikethrough() { insert_wrap(QStringLiteral("~~"), QStringLiteral("~~")); }
void mde::onActionInlineCode()    { insert_wrap(QStringLiteral("`"),  QStringLiteral("`"));  }

void mde::onActionHeading1() { apply_heading(1); }
void mde::onActionHeading2() { apply_heading(2); }
void mde::onActionHeading3() { apply_heading(3); }
void mde::onActionHeading4() { apply_heading(4); }
void mde::onActionHeading5() { apply_heading(5); }
void mde::onActionHeading6() { apply_heading(6); }

void mde::onActionBulletList()  { insert_line_prefix(QStringLiteral("- "));     }
void mde::onActionOrderedList() { insert_line_prefix(QStringLiteral("1. "));    }
void mde::onActionTaskList()    { insert_line_prefix(QStringLiteral("- [ ] ")); }
void mde::onActionBlockquote()  { insert_line_prefix(QStringLiteral("> "));     }

void mde::onActionCodeBlock()
{
    QTextCursor cursor = ui->editorWidget->textCursor();
    const QString selected = cursor.selectedText();
    cursor.insertText(QStringLiteral("```\n") + selected + QStringLiteral("\n```"));
}

void mde::onActionInsertLink()
{
    bool ok = false;
    const QString url = QInputDialog::getText(
        this,
        QStringLiteral("插入链接"),
        QStringLiteral("链接 URL："),
        QLineEdit::Normal, QString(), &ok
    );
    if (!ok || url.isEmpty()) return;

    QTextCursor cursor = ui->editorWidget->textCursor();
    const QString text = cursor.hasSelection()
                         ? cursor.selectedText()
                         : QStringLiteral("链接文字");
    cursor.insertText(QStringLiteral("[") + text
                      + QStringLiteral("](") + url + QStringLiteral(")"));
}

void mde::onActionInsertImage()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("选择图片"),
        QString(),
        QStringLiteral("图片文件 (*.png *.jpg *.jpeg *.gif *.bmp *.svg)")
    );
    if (path.isEmpty()) return;

    QTextCursor cursor = ui->editorWidget->textCursor();
    const QString alt = cursor.hasSelection()
                        ? cursor.selectedText()
                        : QStringLiteral("图片");
    cursor.insertText(QStringLiteral("![") + alt
                      + QStringLiteral("](") + path + QStringLiteral(")"));
}

void mde::onActionInsertTable()
{
    QTextCursor cursor = ui->editorWidget->textCursor();
    cursor.insertText(
        QStringLiteral("| 列1 | 列2 | 列3 |\n"
                        "|-----|-----|-----|\n"
                        "| 内容 | 内容 | 内容 |\n")
    );
}

void mde::onActionHorizontalRule()
{
    QTextCursor cursor = ui->editorWidget->textCursor();
    cursor.movePosition(QTextCursor::EndOfLine);
    cursor.insertText(QStringLiteral("\n\n---\n\n"));
}

// ============================================================
//  视图操作槽
// ============================================================

void mde::onActionToggleSidebar(bool checked)
{
    ui->sidebarDock->setVisible(checked);
}

void mde::onActionTogglePreview(bool checked)
{
    ui->previewWidget->setVisible(checked);
}

void mde::onActionFocusMode(bool checked)
{
    if (checked) {
        ui->sidebarDock->hide();
        ui->previewWidget->hide();
        ui->actionToggleSidebar->setChecked(false);
        ui->actionTogglePreview->setChecked(false);
    } else {
        ui->sidebarDock->show();
        ui->previewWidget->show();
        ui->actionToggleSidebar->setChecked(true);
        ui->actionTogglePreview->setChecked(true);
    }
}

void mde::onActionFullscreen(bool checked)
{
    checked ? showFullScreen() : showNormal();
}

void mde::onActionZoomIn()
{
    ui->editorWidget->zoomIn(2);
}

void mde::onActionZoomOut()
{
    ui->editorWidget->zoomOut(2);
}

void mde::onActionZoomReset()
{
    QFont font = ui->editorWidget->font();
    font.setPointSize(m_base_font_size);
    ui->editorWidget->setFont(font);
}

// ============================================================
//  帮助操作槽
// ============================================================

void mde::onActionShortcuts()
{
    QMessageBox::information(this,
        QStringLiteral("快捷键说明"),
        QStringLiteral(
            "Ctrl+N          新建文件\n"
            "Ctrl+O          打开文件\n"
            "Ctrl+S          保存文件\n"
            "Ctrl+Shift+S    另存为\n"
            "Ctrl+B          加粗\n"
            "Ctrl+I          斜体\n"
            "Ctrl+K          插入链接\n"
            "Ctrl+1~6        标题 H1–H6\n"
            "Ctrl+Shift+U    无序列表\n"
            "Ctrl+Shift+O    有序列表\n"
            "Ctrl+Shift+Q    引用块\n"
            "Ctrl+Shift+K    代码块\n"
            "Ctrl+F          查找\n"
            "Ctrl+\\         切换侧边栏\n"
            "Ctrl+Shift+P    切换预览\n"
            "F11             焦点模式\n"
            "F12             全屏"
        )
    );
}

void mde::onActionAbout()
{
    QMessageBox::about(this,
        QStringLiteral("关于 MDE"),
        QStringLiteral(
            "MDE - Markdown 编辑器\n\n"
            "版本：1.0\n"
            "基于 Qt 5.15 构建\n\n"
            "支持 Markdown 实时预览、语法高亮、大纲导航。"
        )
    );
}

// ============================================================
//  主题
// ============================================================

void mde::setup_theme_group()
{
    /* QActionGroup 保证四个主题 action 互斥勾选 */
    m_theme_group = new QActionGroup(this);
    m_theme_group->setExclusive(true);
    m_theme_group->addAction(ui->actionThemeLight);
    m_theme_group->addAction(ui->actionThemeDark);
    m_theme_group->addAction(ui->actionThemeSepia);
    m_theme_group->addAction(ui->actionThemeHighContrast);

    ThemeManager::load_saved();
    sync_theme_actions();
}

void mde::onActionThemeTriggered()
{
    const QAction *src = qobject_cast<QAction *>(sender());
    if (!src) return;

    Theme theme = Theme::Light;
    if      (src == ui->actionThemeDark)         theme = Theme::Dark;
    else if (src == ui->actionThemeSepia)        theme = Theme::Sepia;
    else if (src == ui->actionThemeHighContrast) theme = Theme::HighContrast;

    ThemeManager::apply(theme);
    sync_theme_actions();

    /* 立即用新主题重新渲染预览 */
    const QString content = ui->editorWidget->toPlainText();
    if (!content.isEmpty())
        emit requestRender(content);
}

void mde::sync_theme_actions()
{
    const Theme t = ThemeManager::current();
    ui->actionThemeLight->setChecked(t == Theme::Light);
    ui->actionThemeDark->setChecked(t == Theme::Dark);
    ui->actionThemeSepia->setChecked(t == Theme::Sepia);
    ui->actionThemeHighContrast->setChecked(t == Theme::HighContrast);
}

// ============================================================
//  内部协调槽
// ============================================================

void mde::onEditorTextChanged()
{
    if (!m_is_modified) {
        set_modified(true);
    }
    /* 重置防抖定时器：连续输入时不断推迟渲染，停顿 300 ms 后触发 */
    m_render_timer->start();
}

void mde::onRenderDebounceTimeout()
{
    const QString content = ui->editorWidget->toPlainText();
    emit requestRender(content);     /* 发送给渲染工作线程 */
    update_outline(content);         /* 大纲解析足够快，直接在 GUI 线程执行 */
}

void mde::onCursorPositionChanged()
{
    update_status_bar();
}

void mde::onRenderFinished(const QString &html)
{
    /* 保存滚动位置，避免更新 HTML 后预览区跳回顶部 */
    const int scroll_pos = ui->previewWidget->verticalScrollBar()->value();
    /* 2026-03-20 新增：注入主题 CSS，使预览区配色与编辑器保持一致 */
    const QString styled = QStringLiteral("<style>")
                         + ThemeManager::preview_css()
                         + QStringLiteral("</style>")
                         + html;
    ui->previewWidget->setHtml(styled);
    ui->previewWidget->verticalScrollBar()->setValue(scroll_pos);
}

void mde::onFileLoaded(const QString &content, const QString &path)
{
    set_editor_content(content);
    m_current_file = path;
    set_modified(false);   /* set_modified 已调用 update_title */
    statusBar()->showMessage(QStringLiteral("已打开：") + path, 3000);
}

void mde::onFileSaved(const QString &path)
{
    m_current_file = path;
    set_modified(false);   /* set_modified 已调用 update_title */
    statusBar()->showMessage(QStringLiteral("已保存：") + path, 3000);
}

void mde::onExportFinished(const QString &path)
{
    statusBar()->showMessage(QStringLiteral("已导出：") + path, 3000);
}

void mde::onWorkerError(const QString &msg)
{
    QMessageBox::warning(this, QStringLiteral("操作失败"), msg);
}

void mde::onOutlineItemClicked(QTreeWidgetItem *item, int /*column*/)
{
    const int line = item->data(0, Qt::UserRole).toInt();
    const QTextBlock block =
        ui->editorWidget->document()->findBlockByLineNumber(line);
    if (block.isValid()) {
        QTextCursor cursor(block);
        ui->editorWidget->setTextCursor(cursor);
        ui->editorWidget->centerCursor();
        ui->editorWidget->setFocus();
    }
}

// ============================================================
//  窗口事件
// ============================================================

void mde::closeEvent(QCloseEvent *event)
{
    confirm_save() ? event->accept() : event->ignore();
}

// ============================================================
//  编辑器格式化辅助
// ============================================================

void mde::insert_wrap(const QString &prefix, const QString &suffix)
{
    QTextCursor cursor = ui->editorWidget->textCursor();
    if (cursor.hasSelection()) {
        /* 包裹已选文字 */
        cursor.insertText(prefix + cursor.selectedText() + suffix);
    } else {
        /* 插入标记并将光标置于中间 */
        const int pos = cursor.position();
        cursor.insertText(prefix + suffix);
        cursor.setPosition(pos + prefix.length());
        ui->editorWidget->setTextCursor(cursor);
    }
}

void mde::insert_line_prefix(const QString &prefix)
{
    QTextCursor cursor = ui->editorWidget->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.insertText(prefix);
    cursor.endEditBlock();
}

void mde::apply_heading(int level)
{
    static const QRegularExpression heading_prefix_re(QStringLiteral("^#{1,6}\\s+"));

    QTextCursor cursor = ui->editorWidget->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

    /* 去掉已有标题前缀，再添加新级别的前缀 */
    QString line = cursor.selectedText();
    line.remove(heading_prefix_re);
    cursor.insertText(QString(level, QLatin1Char('#')) + QLatin1Char(' ') + line);
    cursor.endEditBlock();
}

// ============================================================
//  状态管理辅助
// ============================================================

void mde::update_title()
{
    const QString name = m_current_file.isEmpty()
                         ? QStringLiteral("未命名")
                         : QFileInfo(m_current_file).fileName();
    setWindowTitle(name
                   + (m_is_modified ? QStringLiteral(" *") : QString())
                   + QStringLiteral(" - MDE"));
}

void mde::update_outline(const QString &content)
{
    const QList<OutlineItem> items = OutlineParser::parse(content);

    ui->outlineTreeWidget->clear();

    /* 维护各级别最近一个 QTreeWidgetItem，用于构建层级树 */
    QTreeWidgetItem *last_at_level[7] = { nullptr }; /* 索引 1–6 */

    for (const OutlineItem &item : items) {
        QTreeWidgetItem *node = new QTreeWidgetItem();
        node->setText(0, item.title);
        node->setData(0, Qt::UserRole, item.line_number);

        /* 找最近的父级（比当前级别浅的最后一个节点） */
        QTreeWidgetItem *parent = nullptr;
        for (int i = item.level - 1; i >= 1; --i) {
            if (last_at_level[i]) {
                parent = last_at_level[i];
                break;
            }
        }

        if (parent) {
            parent->addChild(node);
        } else {
            ui->outlineTreeWidget->addTopLevelItem(node);
        }

        last_at_level[item.level] = node;
        /* 清除比当前级别更深的缓存（防止错误挂载） */
        for (int i = item.level + 1; i <= 6; ++i) {
            last_at_level[i] = nullptr;
        }

        node->setExpanded(true);
    }
}

void mde::update_status_bar()
{
    const QTextCursor cursor = ui->editorWidget->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int col  = cursor.columnNumber() + 1;
    statusBar()->showMessage(
        QStringLiteral("行 %1  列 %2").arg(line).arg(col)
    );
}

void mde::set_modified(bool val)
{
    m_is_modified = val;
    update_title();
}

bool mde::confirm_save()
{
    if (!m_is_modified) return true;

    const QMessageBox::StandardButton btn = QMessageBox::question(
        this,
        QStringLiteral("未保存的更改"),
        QStringLiteral("文档有未保存的更改，是否保存？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );

    if (btn == QMessageBox::Save) {
        onActionSave();
        /* 保存是异步的（工作线程），此处乐观地认为已保存 */
        return true;
    }
    return btn == QMessageBox::Discard;
}

void mde::clear_document()
{
    set_editor_content(QString());
    m_current_file.clear();
    set_modified(false);   /* set_modified 已调用 update_title */
}

void mde::setup_icons()
{
    struct IconEntry { QAction *action; const char *path; };
    const IconEntry entries[] = {
        { ui->actionNew,            ":/icons/new.svg"            },
        { ui->actionOpen,           ":/icons/open.svg"           },
        { ui->actionSave,           ":/icons/save.svg"           },
        { ui->actionSaveAs,         ":/icons/save-as.svg"        },
        { ui->actionBold,           ":/icons/bold.svg"           },
        { ui->actionItalic,         ":/icons/italic.svg"         },
        { ui->actionStrikethrough,  ":/icons/strikethrough.svg"  },
        { ui->actionInlineCode,     ":/icons/inline-code.svg"    },
        { ui->actionHeading1,       ":/icons/h1.svg"             },
        { ui->actionHeading2,       ":/icons/h2.svg"             },
        { ui->actionHeading3,       ":/icons/h3.svg"             },
        { ui->actionBulletList,     ":/icons/bullet-list.svg"    },
        { ui->actionOrderedList,    ":/icons/ordered-list.svg"   },
        { ui->actionTaskList,       ":/icons/task-list.svg"      },
        { ui->actionBlockquote,     ":/icons/blockquote.svg"     },
        { ui->actionCodeBlock,      ":/icons/code-block.svg"     },
        { ui->actionInsertLink,     ":/icons/link.svg"           },
        { ui->actionInsertImage,    ":/icons/image.svg"          },
        { ui->actionInsertTable,    ":/icons/table.svg"          },
        { ui->actionHorizontalRule, ":/icons/horizontal-rule.svg"},
        { ui->actionToggleSidebar,  ":/icons/sidebar.svg"        },
        { ui->actionTogglePreview,  ":/icons/preview.svg"        },
    };
    for (const IconEntry &e : entries) {
        e.action->setIcon(QIcon(QString::fromLatin1(e.path)));
    }
}

void mde::set_editor_content(const QString &content)
{
    /* 屏蔽 textChanged 信号，避免设置内容时误触发 modified 标记 */
    ui->editorWidget->blockSignals(true);
    ui->editorWidget->setPlainText(content);
    ui->editorWidget->blockSignals(false);

    /* 内容加载完成后立即触发一次渲染 */
    if (!content.isEmpty()) {
        emit requestRender(content);
        update_outline(content);
    } else {
        ui->previewWidget->clear();
        ui->outlineTreeWidget->clear();
    }
}
