/*
 * 文件 : theme_manager.h
 * 描述 : 应用程序主题管理器（静态工具类）
 * 版本 : v1.0
 * 日期 : 2026-03-20
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   dev      26/03/20   创建文件，提供亮色/暗色/护眼/高对比度四种主题
 */
#pragma once

#include <QString>

/**
 * @brief 主题枚举
 */
enum class Theme {
    Light,        ///< 默认亮色
    Dark,         ///< 暗色模式
    Sepia,        ///< 护眼米黄
    HighContrast  ///< 高对比度
};

/**
 * @brief 主题管理器
 *
 * 纯静态工具类，通过 QSS 对整个应用程序应用主题配色。
 * 使用 QSettings 持久化用户选择，下次启动时自动还原。
 */
class ThemeManager {
public:
    ThemeManager() = delete;

    /**
     * @brief 应用指定主题（设置全局 QSS 并保存到 QSettings）
     * @param theme 目标主题
     */
    static void apply(Theme theme);

    /** @brief 从 QSettings 加载并应用上次保存的主题，应在主窗口显示前调用 */
    static void load_saved();

    /** @brief 返回当前激活的主题 */
    static Theme current();

    /** @brief 返回主题的中文显示名称 */
    static QString display_name(Theme theme);

    /** @brief 主题枚举 → 存储键字符串 */
    static QString to_key(Theme theme);

    /** @brief 存储键字符串 → 主题枚举（未知键返回 Light） */
    static Theme from_key(const QString &key);

    /**
     * @brief 返回注入到 HTML 预览中的 CSS 片段（随当前主题变化）
     * @note  在 setHtml() 前拼入 <style>...</style> 块
     */
    static QString preview_css();

private:
    static Theme   m_current;

    static QString qss_light();
    static QString qss_dark();
    static QString qss_sepia();
    static QString qss_high_contrast();
};
