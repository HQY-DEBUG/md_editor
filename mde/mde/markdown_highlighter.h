/*
 * 文件 : markdown_highlighter.h
 * 描述 : Markdown 语法高亮器
 * 版本 : v1.1
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.1   dev      26/03/19   新增代码块语言语法高亮，通过 CodeBlockData 跨块传递语言信息
 * 1.0   dev      26/03/19   创建文件
 */
#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextBlockUserData>
#include <QRegularExpression>
#include <QVector>

/**
 * @brief 代码块块用户数据
 *
 * 附加在代码块（包括围栏行及每行内容）所在的 QTextBlock 上，
 * 记录该代码块使用的语言，供后续行继承。
 */
class CodeBlockData : public QTextBlockUserData {
public:
    QString language; ///< 规范化语言名（如 "cpp"、"python"），空表示未标注语言
};

/**
 * @brief Markdown 语法高亮器
 *
 * 继承 QSyntaxHighlighter，为编辑器提供 Markdown 语法着色。
 * 支持：
 *  - 标题、粗体、斜体、删除线、行内代码、链接、引用、列表、分隔线
 *  - 多行代码块（状态机：0 = 普通 Markdown，1 = 代码块内）
 *  - 代码块内按语言进行语法高亮（借助 CodeHighlighter）
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
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat    format;
    };

    QVector<HighlightRule> m_rules;             ///< Markdown 单行高亮规则列表
    QRegularExpression     m_code_fence_re;     ///< 匹配代码块围栏行（行首 ```）
    QTextCharFormat        m_code_block_format; ///< 代码块基础格式（背景+等宽字体）

    /**
     * @brief 初始化 Markdown 高亮规则（构造函数中调用一次）
     */
    void setup_rules();

    /**
     * @brief 对代码块内的一行应用语言语法高亮（叠加在基础格式之上）
     * @param text 行文本
     * @param lang 规范化语言名（空则不高亮）
     */
    void highlight_code(const QString &text, const QString &lang);
};
