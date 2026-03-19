/*
 * 文件 : outline_parser.h
 * 描述 : Markdown 大纲解析工具类
 * 版本 : v1.0
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   dev      26/03/19   创建文件
 */
#pragma once

#include <QString>
#include <QList>

/**
 * @brief 大纲条目，表示 Markdown 中的一个标题
 */
struct OutlineItem {
    int     level;       ///< 标题级别（1–6 对应 H1–H6）
    QString title;       ///< 标题文字（去除前缀 # 和空格）
    int     line_number; ///< 在源文件中的行号（0 起始）
};

/**
 * @brief Markdown 大纲解析器（纯静态工具类，不继承 QObject）
 *
 * 从 Markdown 文本中提取所有标题行，构建大纲条目列表。
 * 速度足够快，可直接在 GUI 线程调用，无需工作线程。
 */
class OutlineParser {
public:
    OutlineParser() = delete; // 禁止实例化，仅用静态方法

    /**
     * @brief 解析 Markdown 文本中的所有标题
     * @param  markdown 原始 Markdown 内容
     * @return 按行号排列的大纲条目列表
     */
    static QList<OutlineItem> parse(const QString &markdown);
};
