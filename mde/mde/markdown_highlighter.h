/*
 * 文件 : markdown_highlighter.h
 * 描述 : Markdown 语法高亮器
 * 版本 : v1.0
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   dev      26/03/19   创建文件
 */
#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

/**
 * @brief Markdown 语法高亮器
 *
 * 继承 QSyntaxHighlighter，为编辑器提供 Markdown 语法着色。
 * 支持标题、粗体、斜体、删除线、行内代码、多行代码块、链接、引用等语法。
 */
class MarkdownHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr);

protected:
    /**
     * @brief 对单行文本执行高亮（由 QSyntaxHighlighter 框架调用）
     * @param text 当前块的文本内容
     */
    void highlightBlock(const QString &text) override;

private:
    // --- 内部数据结构 ---//
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat    format;
    };

    QVector<HighlightRule> m_rules;            ///< 单行高亮规则列表

    QRegularExpression     m_code_block_start; ///< 代码块起始标记（```）
    QRegularExpression     m_code_block_end;   ///< 代码块结束标记（```）
    QTextCharFormat        m_code_block_format;///< 代码块内容格式

    /**
     * @brief 初始化所有高亮规则（在构造函数中调用一次）
     */
    void setup_rules();
};
