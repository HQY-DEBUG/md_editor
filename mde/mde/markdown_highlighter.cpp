/*
 * 文件 : markdown_highlighter.cpp
 * 描述 : Markdown 语法高亮器实现
 * 版本 : v1.0
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   dev      26/03/19   创建文件
 */
#include "markdown_highlighter.h"

#include <QFont>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_code_block_start(QStringLiteral("^```"))
    , m_code_block_end(QStringLiteral("^```"))
{
    setup_rules();
}

// --- 初始化 ---//

void MarkdownHighlighter::setup_rules()
{
    // --- 标题 H1–H6（行首 # 号） ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("^#{1,6}\\s+.*$"));
        rule.format.setForeground(QColor(0x2E, 0x86, 0xC1));
        rule.format.setFontWeight(QFont::Bold);
        m_rules.append(rule);
    }

    // --- 粗体 **text** 或 __text__ ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("(\\*\\*|__)(.+?)\\1"));
        rule.format.setFontWeight(QFont::Bold);
        m_rules.append(rule);
    }

    // --- 斜体 *text* 或 _text_（排除粗体的双符号） ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("(?<!\\*)(\\*|_)(?!\\1)(.+?)\\1(?!\\1)"));
        rule.format.setFontItalic(true);
        m_rules.append(rule);
    }

    // --- 删除线 ~~text~~ ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("~~(.+?)~~"));
        rule.format.setFontStrikeOut(true);
        rule.format.setForeground(Qt::gray);
        m_rules.append(rule);
    }

    // --- 行内代码 `code`（优先级高，防止内部再匹配其他规则） ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("`[^`\n]+`"));
        rule.format.setFontFamily(QStringLiteral("Consolas"));
        rule.format.setForeground(QColor(0xC0, 0x39, 0x2B));
        rule.format.setBackground(QColor(0xF5, 0xF5, 0xF5));
        m_rules.append(rule);
    }

    // --- 链接 [text](url) ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("!?\\[.*?\\]\\(.*?\\)"));
        rule.format.setForeground(QColor(0x27, 0xAE, 0x60));
        rule.format.setFontUnderline(true);
        m_rules.append(rule);
    }

    // --- 引用块 > text ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("^>.*$"));
        rule.format.setForeground(QColor(0x7F, 0x8C, 0x8D));
        rule.format.setFontItalic(true);
        m_rules.append(rule);
    }

    // --- 无序列表 - / * / + ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("^(\\s*)([-*+])\\s"));
        rule.format.setForeground(QColor(0xE6, 0x7E, 0x22));
        rule.format.setFontWeight(QFont::Bold);
        m_rules.append(rule);
    }

    // --- 有序列表 1. ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("^\\s*\\d+\\.\\s"));
        rule.format.setForeground(QColor(0xE6, 0x7E, 0x22));
        m_rules.append(rule);
    }

    // --- 任务列表 - [ ] / - [x] ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("^\\s*-\\s\\[(x| )\\]\\s"));
        rule.format.setForeground(QColor(0x8E, 0x44, 0xAD));
        m_rules.append(rule);
    }

    // --- 分隔线 --- / *** / ___ ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("^(---+|\\*\\*\\*+|___+)\\s*$"));
        rule.format.setForeground(Qt::gray);
        m_rules.append(rule);
    }

    // --- 代码块标记行（``` 或 ```lang）的格式 ---//
    m_code_block_format.setFontFamily(QStringLiteral("Consolas"));
    m_code_block_format.setForeground(QColor(0x2C, 0x3E, 0x50));
    m_code_block_format.setBackground(QColor(0xEC, 0xF0, 0xF1));
}

// --- 高亮逻辑 ---//

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    // --- 多行代码块处理（状态机：0 = 正常，1 = 代码块内） ---//
    setCurrentBlockState(0);

    int start_index = 0;
    if (previousBlockState() != 1) {
        /* 未在代码块内，查找 ``` 起始 */
        start_index = text.indexOf(m_code_block_start);
    }

    while (start_index >= 0) {
        /* 从 ``` 起始后查找结束位置 */
        QRegularExpressionMatch end_match =
            m_code_block_end.match(text, start_index + 3);

        int code_len;
        if (!end_match.hasMatch()) {
            /* 未找到结束符：本行及后续行均在代码块内 */
            setCurrentBlockState(1);
            code_len = text.length() - start_index;
        } else {
            code_len = end_match.capturedStart() - start_index
                       + end_match.capturedLength();
        }

        setFormat(start_index, code_len, m_code_block_format);
        start_index = text.indexOf(m_code_block_start, start_index + code_len);
    }

    /* 若整行处于代码块内，直接设置格式后返回，不再匹配其他规则 */
    if (previousBlockState() == 1) {
        setFormat(0, text.length(), m_code_block_format);
        return;
    }

    // --- 按顺序应用单行高亮规则 ---//
    for (const HighlightRule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
