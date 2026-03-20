/*
 * 文件 : markdown_highlighter.cpp
 * 描述 : Markdown 语法高亮器实现
 * 版本 : v1.1
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.1   dev      26/03/19   新增代码块语言高亮，改用 CodeBlockData 跨块传递语言信息
 * 1.0   dev      26/03/19   创建文件
 */
#include "markdown_highlighter.h"
#include "code_highlighter.h"

#include <QFont>
#include <QTextBlock>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_code_fence_re(QStringLiteral("^```"))
{
    setup_rules();
}

// ============================================================
//  初始化
// ============================================================

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

    // --- 行内代码 `code` ---//
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("`[^`\n]+`"));
        rule.format.setFontFamily(QStringLiteral("Consolas"));
        rule.format.setForeground(QColor(0xC0, 0x39, 0x2B));
        rule.format.setBackground(QColor(0xF5, 0xF5, 0xF5));
        m_rules.append(rule);
    }

    // --- 链接 [text](url) 和图片 ![alt](url) ---//
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

    // --- 代码块基础格式（等宽字体 + 浅灰背景） ---//
    m_code_block_format.setFontFamily(QStringLiteral("Consolas"));
    m_code_block_format.setForeground(QColor(0x2C, 0x3E, 0x50));
    m_code_block_format.setBackground(QColor(0xEC, 0xF0, 0xF1));
}

// ============================================================
//  高亮逻辑
// ============================================================

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    setCurrentBlockState(0);

    /* ── 当前行处于多行代码块内部（前一块状态为 1）── */
    if (previousBlockState() == 1) {
        /* 从前一块的用户数据继承语言信息 */
        QString lang;
        if (auto *prev_data = static_cast<CodeBlockData *>(
                currentBlock().previous().userData())) {
            lang = prev_data->language;
        }

        /* 将语言信息附到当前块，供下一行继续继承 */
        auto *data = new CodeBlockData();
        data->language = lang;
        setCurrentBlockUserData(data);

        if (m_code_fence_re.match(text).hasMatch()) {
            /* 代码块结束围栏行（```） */
            setFormat(0, text.length(), m_code_block_format);
            setCurrentBlockState(0);
            return;
        }

        /* 仍在代码块内：先应用基础格式（背景+字体），再叠加语言高亮 */
        setCurrentBlockState(1);
        setFormat(0, text.length(), m_code_block_format);
        highlight_code(text, lang);
        return;
    }

    /* ── 检测代码块开始围栏行（行首 ```[lang]）── */
    if (m_code_fence_re.match(text).hasMatch()) {
        /* 提取语言标识符（取第一个词，小写规范化） */
        QString raw = text.mid(3).trimmed().toLower();
        const int sp = raw.indexOf(QLatin1Char(' '));
        if (sp > 0) raw = raw.left(sp);

        auto *data = new CodeBlockData();
        data->language = CodeHighlighter::normalize(raw);
        setCurrentBlockUserData(data);

        setFormat(0, text.length(), m_code_block_format);
        setCurrentBlockState(1);
        return;
    }

    /* ── 普通 Markdown 行：按顺序应用单行规则 ── */
    for (const HighlightRule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

void MarkdownHighlighter::highlight_code(const QString &text, const QString &lang)
{
    if (lang.isEmpty()) return;

    const QVector<CodeRule> &rules = CodeHighlighter::rules_for(lang);
    for (const CodeRule &rule : rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
