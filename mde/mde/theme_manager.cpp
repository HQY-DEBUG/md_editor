/*
 * 文件 : theme_manager.cpp
 * 描述 : 应用程序主题管理器实现
 * 版本 : v1.0
 * 日期 : 2026-03-20
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   dev      26/03/20   创建文件
 */
#include "theme_manager.h"

#include <QApplication>
#include <QSettings>

// --- 静态成员初始化 ---//
Theme ThemeManager::m_current = Theme::Light;

// ============================================================
//  公开接口
// ============================================================

void ThemeManager::apply(Theme theme)
{
    m_current = theme;

    switch (theme) {
    case Theme::Light:        qApp->setStyleSheet(qss_light());        break;
    case Theme::Dark:         qApp->setStyleSheet(qss_dark());         break;
    case Theme::Sepia:        qApp->setStyleSheet(qss_sepia());        break;
    case Theme::HighContrast: qApp->setStyleSheet(qss_high_contrast()); break;
    }

    /* 持久化 */
    QSettings settings(QStringLiteral("MDE"), QStringLiteral("Preferences"));
    settings.setValue(QStringLiteral("theme"), to_key(theme));
}

void ThemeManager::load_saved()
{
    QSettings settings(QStringLiteral("MDE"), QStringLiteral("Preferences"));
    const QString key = settings.value(QStringLiteral("theme"),
                                       QStringLiteral("light")).toString();
    apply(from_key(key));
}

Theme ThemeManager::current()
{
    return m_current;
}

QString ThemeManager::display_name(Theme theme)
{
    switch (theme) {
    case Theme::Light:        return QStringLiteral("亮色（默认）");
    case Theme::Dark:         return QStringLiteral("暗色");
    case Theme::Sepia:        return QStringLiteral("护眼米黄");
    case Theme::HighContrast: return QStringLiteral("高对比度");
    }
    return QStringLiteral("亮色（默认）");
}

QString ThemeManager::to_key(Theme theme)
{
    switch (theme) {
    case Theme::Light:        return QStringLiteral("light");
    case Theme::Dark:         return QStringLiteral("dark");
    case Theme::Sepia:        return QStringLiteral("sepia");
    case Theme::HighContrast: return QStringLiteral("high_contrast");
    }
    return QStringLiteral("light");
}

Theme ThemeManager::from_key(const QString &key)
{
    if (key == QStringLiteral("dark"))          return Theme::Dark;
    if (key == QStringLiteral("sepia"))         return Theme::Sepia;
    if (key == QStringLiteral("high_contrast")) return Theme::HighContrast;
    return Theme::Light;
}

QString ThemeManager::preview_css()
{
    switch (m_current) {
    case Theme::Dark:
        return QStringLiteral(
            "body { background-color:#1e1e1e; color:#d4d4d4; font-family:sans-serif; padding:12px; }"
            "h1,h2,h3,h4,h5,h6 { color:#569cd6; border-bottom:1px solid #3c3c3c; padding-bottom:4px; }"
            "a { color:#4ec9b0; }"
            "code { background:#2d2d2d; color:#ce9178; padding:1px 4px; border-radius:3px; }"
            "pre  { background:#2d2d2d; color:#d4d4d4; padding:10px; border-radius:4px; overflow:auto; }"
            "blockquote { border-left:4px solid #3c3c3c; margin:0; padding:0 12px; color:#999; }"
            "table { border-collapse:collapse; width:100%; }"
            "th,td { border:1px solid #3c3c3c; padding:6px 10px; }"
            "th { background:#2d2d2d; }"
            "tr:nth-child(even) { background:#252526; }"
            "hr { border:none; border-top:1px solid #3c3c3c; }"
        );
    case Theme::Sepia:
        return QStringLiteral(
            "body { background-color:#faf3e0; color:#4a3728; font-family:serif; padding:12px; }"
            "h1,h2,h3,h4,h5,h6 { color:#6b3f0e; border-bottom:1px solid #c8aa80; padding-bottom:4px; }"
            "a { color:#8b6914; }"
            "code { background:#ece0b8; color:#7a3b00; padding:1px 4px; border-radius:3px; }"
            "pre  { background:#ece0b8; color:#4a3728; padding:10px; border-radius:4px; overflow:auto; }"
            "blockquote { border-left:4px solid #c8aa80; margin:0; padding:0 12px; color:#7a6550; }"
            "table { border-collapse:collapse; width:100%; }"
            "th,td { border:1px solid #c8aa80; padding:6px 10px; }"
            "th { background:#ece0b8; }"
            "tr:nth-child(even) { background:#f4e8c8; }"
            "hr { border:none; border-top:1px solid #c8aa80; }"
        );
    case Theme::HighContrast:
        return QStringLiteral(
            "body { background-color:#000000; color:#ffffff; font-family:sans-serif; padding:12px; }"
            "h1,h2,h3,h4,h5,h6 { color:#1aebff; border-bottom:1px solid #ffffff; padding-bottom:4px; }"
            "a { color:#ffff00; text-decoration:underline; }"
            "code { background:#1a1a1a; color:#1aebff; padding:1px 4px; border:1px solid #ffffff; border-radius:3px; }"
            "pre  { background:#1a1a1a; color:#ffffff; padding:10px; border:1px solid #ffffff; border-radius:4px; overflow:auto; }"
            "blockquote { border-left:4px solid #ffffff; margin:0; padding:0 12px; color:#cccccc; }"
            "table { border-collapse:collapse; width:100%; }"
            "th,td { border:1px solid #ffffff; padding:6px 10px; }"
            "th { background:#1a1a1a; }"
            "tr:nth-child(even) { background:#0d0d0d; }"
            "hr { border:none; border-top:1px solid #ffffff; }"
        );
    default: /* Light */
        return QStringLiteral(
            "body { background-color:#ffffff; color:#1e1e1e; font-family:sans-serif; padding:12px; }"
            "h1,h2,h3,h4,h5,h6 { color:#0078d4; border-bottom:1px solid #dcdcdc; padding-bottom:4px; }"
            "a { color:#0078d4; }"
            "code { background:#f0f0f0; color:#c0392b; padding:1px 4px; border-radius:3px; }"
            "pre  { background:#f5f5f5; color:#1e1e1e; padding:10px; border-radius:4px; overflow:auto; }"
            "blockquote { border-left:4px solid #dcdcdc; margin:0; padding:0 12px; color:#666; }"
            "table { border-collapse:collapse; width:100%; }"
            "th,td { border:1px solid #dcdcdc; padding:6px 10px; }"
            "th { background:#f5f5f5; }"
            "tr:nth-child(even) { background:#fafafa; }"
            "hr { border:none; border-top:1px solid #dcdcdc; }"
        );
    }
}

// ============================================================
//  QSS 主题字符串
// ============================================================

QString ThemeManager::qss_light()
{
    return QStringLiteral(
        /* ── 基础 ── */
        "QWidget { background-color:#ffffff; color:#1e1e1e; }"
        /* ── 菜单栏 ── */
        "QMenuBar { background-color:#f8f8f8; color:#1e1e1e; border-bottom:1px solid #dcdcdc; }"
        "QMenuBar::item { background:transparent; padding:4px 10px; }"
        "QMenuBar::item:selected { background-color:#e5f3ff; }"
        "QMenuBar::item:pressed  { background-color:#cce4ff; }"
        /* ── 菜单 ── */
        "QMenu { background-color:#ffffff; color:#1e1e1e; border:1px solid #dcdcdc; }"
        "QMenu::item { padding:5px 24px 5px 16px; }"
        "QMenu::item:selected { background-color:#e5f3ff; color:#0078d4; }"
        "QMenu::separator { height:1px; background:#dcdcdc; margin:3px 0; }"
        /* ── 工具栏 ── */
        "QToolBar { background-color:#f0f0f0; border-bottom:1px solid #dcdcdc; spacing:2px; padding:2px; }"
        "QToolBar QToolButton { background:transparent; color:#1e1e1e; border:none; padding:3px 4px; border-radius:3px; }"
        "QToolBar QToolButton:hover   { background-color:#d0e8ff; }"
        "QToolBar QToolButton:pressed { background-color:#b8d8ff; }"
        "QToolBar QToolButton:checked { background-color:#cce4ff; }"
        "QToolBar::separator { width:1px; background:#dcdcdc; margin:4px 2px; }"
        /* ── 状态栏 ── */
        "QStatusBar { background-color:#0078d4; color:#ffffff; }"
        "QStatusBar::item { border:none; }"
        /* ── Dock ── */
        "QDockWidget { background-color:#f5f5f5; }"
        "QDockWidget::title { background-color:#e8e8e8; padding:5px; border-bottom:1px solid #dcdcdc; color:#1e1e1e; }"
        /* ── 标签页 ── */
        "QTabWidget::pane { border:none; background-color:#f5f5f5; }"
        "QTabBar { background-color:#f0f0f0; }"
        "QTabBar::tab { background-color:#e8e8e8; color:#555; padding:6px 14px; border:none; border-right:1px solid #dcdcdc; }"
        "QTabBar::tab:selected { background-color:#ffffff; color:#1e1e1e; border-top:2px solid #0078d4; }"
        "QTabBar::tab:hover:!selected { background-color:#f0f0f0; color:#1e1e1e; }"
        /* ── 编辑器 ── */
        "QPlainTextEdit { background-color:#ffffff; color:#1e1e1e; selection-background-color:#0078d4; selection-color:#ffffff; border:none; }"
        /* ── 预览区 ── */
        "QTextBrowser { background-color:#ffffff; color:#1e1e1e; selection-background-color:#0078d4; border:none; }"
        /* ── 树形控件 ── */
        "QTreeWidget { background-color:#ffffff; color:#1e1e1e; border:none; alternate-background-color:#fafafa; outline:none; }"
        "QTreeWidget::item { padding:2px; }"
        "QTreeWidget::item:hover    { background-color:#e5f3ff; }"
        "QTreeWidget::item:selected { background-color:#0078d4; color:#ffffff; }"
        /* ── 输入框 ── */
        "QLineEdit { background-color:#ffffff; color:#1e1e1e; border:1px solid #dcdcdc; padding:3px 5px; border-radius:2px; }"
        "QLineEdit:focus { border-color:#0078d4; }"
        /* ── 分割条 ── */
        "QSplitter::handle { background-color:#dcdcdc; }"
        "QSplitter::handle:horizontal { width:2px; }"
        "QSplitter::handle:vertical   { height:2px; }"
        /* ── 滚动条 ── */
        "QScrollBar:vertical   { background:#f5f5f5; width:10px; margin:0; }"
        "QScrollBar::handle:vertical   { background:#c0c0c0; min-height:20px; border-radius:5px; }"
        "QScrollBar::handle:vertical:hover { background:#a0a0a0; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:none; }"
        "QScrollBar:horizontal { background:#f5f5f5; height:10px; margin:0; }"
        "QScrollBar::handle:horizontal { background:#c0c0c0; min-width:20px; border-radius:5px; }"
        "QScrollBar::handle:horizontal:hover { background:#a0a0a0; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background:none; }"
    );
}

QString ThemeManager::qss_dark()
{
    return QStringLiteral(
        /* ── 基础 ── */
        "QWidget { background-color:#1e1e1e; color:#d4d4d4; }"
        /* ── 菜单栏 ── */
        "QMenuBar { background-color:#3c3c3c; color:#d4d4d4; border-bottom:1px solid #252525; }"
        "QMenuBar::item { background:transparent; padding:4px 10px; }"
        "QMenuBar::item:selected { background-color:#505050; }"
        "QMenuBar::item:pressed  { background-color:#094771; }"
        /* ── 菜单 ── */
        "QMenu { background-color:#2d2d2d; color:#d4d4d4; border:1px solid #454545; }"
        "QMenu::item { padding:5px 24px 5px 16px; }"
        "QMenu::item:selected { background-color:#094771; color:#ffffff; }"
        "QMenu::separator { height:1px; background:#454545; margin:3px 0; }"
        /* ── 工具栏 ── */
        "QToolBar { background-color:#333333; border-bottom:1px solid #252525; spacing:2px; padding:2px; }"
        "QToolBar QToolButton { background:transparent; color:#d4d4d4; border:none; padding:3px 4px; border-radius:3px; }"
        "QToolBar QToolButton:hover   { background-color:#505050; }"
        "QToolBar QToolButton:pressed { background-color:#094771; }"
        "QToolBar QToolButton:checked { background-color:#1f4b73; }"
        "QToolBar::separator { width:1px; background:#454545; margin:4px 2px; }"
        /* ── 状态栏 ── */
        "QStatusBar { background-color:#007acc; color:#ffffff; }"
        "QStatusBar::item { border:none; }"
        /* ── Dock ── */
        "QDockWidget { background-color:#252526; }"
        "QDockWidget::title { background-color:#3c3c3c; padding:5px; border-bottom:1px solid #252525; color:#d4d4d4; }"
        /* ── 标签页 ── */
        "QTabWidget::pane { border:none; background-color:#252526; }"
        "QTabBar { background-color:#2d2d2d; }"
        "QTabBar::tab { background-color:#2d2d2d; color:#999999; padding:6px 14px; border:none; border-right:1px solid #252525; }"
        "QTabBar::tab:selected { background-color:#1e1e1e; color:#d4d4d4; border-top:2px solid #007acc; }"
        "QTabBar::tab:hover:!selected { background-color:#3c3c3c; color:#d4d4d4; }"
        /* ── 编辑器 ── */
        "QPlainTextEdit { background-color:#1e1e1e; color:#d4d4d4; selection-background-color:#264f78; selection-color:#ffffff; border:none; }"
        /* ── 预览区 ── */
        "QTextBrowser { background-color:#1e1e1e; color:#d4d4d4; selection-background-color:#264f78; border:none; }"
        /* ── 树形控件 ── */
        "QTreeWidget { background-color:#252526; color:#d4d4d4; border:none; alternate-background-color:#2a2a2b; outline:none; }"
        "QTreeWidget::item { padding:2px; }"
        "QTreeWidget::item:hover    { background-color:#2a2d2e; }"
        "QTreeWidget::item:selected { background-color:#094771; color:#ffffff; }"
        /* ── 输入框 ── */
        "QLineEdit { background-color:#3c3c3c; color:#d4d4d4; border:1px solid #454545; padding:3px 5px; border-radius:2px; }"
        "QLineEdit:focus { border-color:#007acc; }"
        /* ── 分割条 ── */
        "QSplitter::handle { background-color:#3c3c3c; }"
        "QSplitter::handle:horizontal { width:2px; }"
        "QSplitter::handle:vertical   { height:2px; }"
        /* ── 滚动条 ── */
        "QScrollBar:vertical   { background:#1e1e1e; width:10px; margin:0; }"
        "QScrollBar::handle:vertical   { background:#424242; min-height:20px; border-radius:5px; }"
        "QScrollBar::handle:vertical:hover { background:#686868; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:none; }"
        "QScrollBar:horizontal { background:#1e1e1e; height:10px; margin:0; }"
        "QScrollBar::handle:horizontal { background:#424242; min-width:20px; border-radius:5px; }"
        "QScrollBar::handle:horizontal:hover { background:#686868; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background:none; }"
    );
}

QString ThemeManager::qss_sepia()
{
    return QStringLiteral(
        /* ── 基础 ── */
        "QWidget { background-color:#faf3e0; color:#4a3728; }"
        /* ── 菜单栏 ── */
        "QMenuBar { background-color:#ece0b8; color:#4a3728; border-bottom:1px solid #c8aa80; }"
        "QMenuBar::item { background:transparent; padding:4px 10px; }"
        "QMenuBar::item:selected { background-color:#d4bc90; }"
        "QMenuBar::item:pressed  { background-color:#c8a878; }"
        /* ── 菜单 ── */
        "QMenu { background-color:#faf3e0; color:#4a3728; border:1px solid #c8aa80; }"
        "QMenu::item { padding:5px 24px 5px 16px; }"
        "QMenu::item:selected { background-color:#d4bc90; color:#2a1a0e; }"
        "QMenu::separator { height:1px; background:#c8aa80; margin:3px 0; }"
        /* ── 工具栏 ── */
        "QToolBar { background-color:#ece0b8; border-bottom:1px solid #c8aa80; spacing:2px; padding:2px; }"
        "QToolBar QToolButton { background:transparent; color:#4a3728; border:none; padding:3px 4px; border-radius:3px; }"
        "QToolBar QToolButton:hover   { background-color:#d4bc90; }"
        "QToolBar QToolButton:pressed { background-color:#c8a878; }"
        "QToolBar QToolButton:checked { background-color:#c8a878; }"
        "QToolBar::separator { width:1px; background:#c8aa80; margin:4px 2px; }"
        /* ── 状态栏 ── */
        "QStatusBar { background-color:#6b4c10; color:#faf3e0; }"
        "QStatusBar::item { border:none; }"
        /* ── Dock ── */
        "QDockWidget { background-color:#f4e8c8; }"
        "QDockWidget::title { background-color:#ece0b8; padding:5px; border-bottom:1px solid #c8aa80; color:#4a3728; }"
        /* ── 标签页 ── */
        "QTabWidget::pane { border:none; background-color:#f4e8c8; }"
        "QTabBar { background-color:#ece0b8; }"
        "QTabBar::tab { background-color:#e8d8b0; color:#7a6550; padding:6px 14px; border:none; border-right:1px solid #c8aa80; }"
        "QTabBar::tab:selected { background-color:#faf3e0; color:#4a3728; border-top:2px solid #8b6914; }"
        "QTabBar::tab:hover:!selected { background-color:#f0e4c8; color:#4a3728; }"
        /* ── 编辑器 ── */
        "QPlainTextEdit { background-color:#faf3e0; color:#4a3728; selection-background-color:#c8a878; selection-color:#2a1a0e; border:none; }"
        /* ── 预览区 ── */
        "QTextBrowser { background-color:#faf3e0; color:#4a3728; selection-background-color:#c8a878; border:none; }"
        /* ── 树形控件 ── */
        "QTreeWidget { background-color:#f4e8c8; color:#4a3728; border:none; alternate-background-color:#faf3e0; outline:none; }"
        "QTreeWidget::item { padding:2px; }"
        "QTreeWidget::item:hover    { background-color:#e8d8b0; }"
        "QTreeWidget::item:selected { background-color:#c8a878; color:#2a1a0e; }"
        /* ── 输入框 ── */
        "QLineEdit { background-color:#faf3e0; color:#4a3728; border:1px solid #c8aa80; padding:3px 5px; border-radius:2px; }"
        "QLineEdit:focus { border-color:#8b6914; }"
        /* ── 分割条 ── */
        "QSplitter::handle { background-color:#c8aa80; }"
        "QSplitter::handle:horizontal { width:2px; }"
        "QSplitter::handle:vertical   { height:2px; }"
        /* ── 滚动条 ── */
        "QScrollBar:vertical   { background:#faf3e0; width:10px; margin:0; }"
        "QScrollBar::handle:vertical   { background:#c8aa80; min-height:20px; border-radius:5px; }"
        "QScrollBar::handle:vertical:hover { background:#a08860; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:none; }"
        "QScrollBar:horizontal { background:#faf3e0; height:10px; margin:0; }"
        "QScrollBar::handle:horizontal { background:#c8aa80; min-width:20px; border-radius:5px; }"
        "QScrollBar::handle:horizontal:hover { background:#a08860; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background:none; }"
    );
}

QString ThemeManager::qss_high_contrast()
{
    return QStringLiteral(
        /* ── 基础 ── */
        "QWidget { background-color:#000000; color:#ffffff; }"
        /* ── 菜单栏 ── */
        "QMenuBar { background-color:#1a1a1a; color:#ffffff; border-bottom:1px solid #ffffff; }"
        "QMenuBar::item { background:transparent; padding:4px 10px; }"
        "QMenuBar::item:selected { background-color:#1aebff; color:#000000; }"
        "QMenuBar::item:pressed  { background-color:#1aebff; color:#000000; }"
        /* ── 菜单 ── */
        "QMenu { background-color:#000000; color:#ffffff; border:1px solid #ffffff; }"
        "QMenu::item { padding:5px 24px 5px 16px; }"
        "QMenu::item:selected { background-color:#1aebff; color:#000000; }"
        "QMenu::separator { height:1px; background:#ffffff; margin:3px 0; }"
        /* ── 工具栏 ── */
        "QToolBar { background-color:#1a1a1a; border-bottom:1px solid #ffffff; spacing:2px; padding:2px; }"
        "QToolBar QToolButton { background:transparent; color:#ffffff; border:none; padding:3px 4px; border-radius:3px; }"
        "QToolBar QToolButton:hover   { background-color:#333333; border:1px solid #ffffff; }"
        "QToolBar QToolButton:pressed { background-color:#1aebff; color:#000000; }"
        "QToolBar QToolButton:checked { background-color:#1aebff; color:#000000; }"
        "QToolBar::separator { width:1px; background:#ffffff; margin:4px 2px; }"
        /* ── 状态栏 ── */
        "QStatusBar { background-color:#1aebff; color:#000000; }"
        "QStatusBar::item { border:none; }"
        /* ── Dock ── */
        "QDockWidget { background-color:#0c0c0c; }"
        "QDockWidget::title { background-color:#1a1a1a; padding:5px; border-bottom:1px solid #ffffff; color:#ffffff; }"
        /* ── 标签页 ── */
        "QTabWidget::pane { border:1px solid #ffffff; background-color:#000000; }"
        "QTabBar { background-color:#1a1a1a; }"
        "QTabBar::tab { background-color:#1a1a1a; color:#aaaaaa; padding:6px 14px; border:1px solid #555555; }"
        "QTabBar::tab:selected { background-color:#000000; color:#ffffff; border-top:2px solid #1aebff; }"
        "QTabBar::tab:hover:!selected { background-color:#333333; color:#ffffff; }"
        /* ── 编辑器 ── */
        "QPlainTextEdit { background-color:#000000; color:#ffffff; selection-background-color:#1aebff; selection-color:#000000; border:1px solid #555555; }"
        /* ── 预览区 ── */
        "QTextBrowser { background-color:#000000; color:#ffffff; selection-background-color:#1aebff; border:1px solid #555555; }"
        /* ── 树形控件 ── */
        "QTreeWidget { background-color:#000000; color:#ffffff; border:1px solid #555555; alternate-background-color:#0d0d0d; outline:none; }"
        "QTreeWidget::item { padding:2px; }"
        "QTreeWidget::item:hover    { background-color:#1a1a1a; border:1px solid #ffffff; }"
        "QTreeWidget::item:selected { background-color:#1aebff; color:#000000; }"
        /* ── 输入框 ── */
        "QLineEdit { background-color:#000000; color:#ffffff; border:1px solid #ffffff; padding:3px 5px; border-radius:2px; }"
        "QLineEdit:focus { border-color:#1aebff; }"
        /* ── 分割条 ── */
        "QSplitter::handle { background-color:#ffffff; }"
        "QSplitter::handle:horizontal { width:2px; }"
        "QSplitter::handle:vertical   { height:2px; }"
        /* ── 滚动条 ── */
        "QScrollBar:vertical   { background:#000000; width:12px; margin:0; border:1px solid #555; }"
        "QScrollBar::handle:vertical   { background:#888888; min-height:20px; border-radius:6px; }"
        "QScrollBar::handle:vertical:hover { background:#ffffff; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:none; }"
        "QScrollBar:horizontal { background:#000000; height:12px; margin:0; border:1px solid #555; }"
        "QScrollBar::handle:horizontal { background:#888888; min-width:20px; border-radius:6px; }"
        "QScrollBar::handle:horizontal:hover { background:#ffffff; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background:none; }"
    );
}
