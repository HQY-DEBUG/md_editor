/*
 * 文件 : outline_parser.cpp
 * 描述 : Markdown 大纲解析工具类实现
 * 版本 : v1.0
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   dev      26/03/19   创建文件
 */
#include "outline_parser.h"

#include <QStringList>
#include <QRegularExpression>

QList<OutlineItem> OutlineParser::parse(const QString &markdown)
{
    QList<OutlineItem> items;
    const QStringList lines = markdown.split(QLatin1Char('\n'));

    /* 匹配 ATX 格式标题：行首 1–6 个 # 后跟空格，其余为标题文字 */
    static const QRegularExpression heading_re(
        QStringLiteral("^(#{1,6})\\s+(.+?)(?:\\s+#+)?\\s*$")
    );

    bool in_code_block = false; /* 跳过代码块内部的 # 行 */

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];

        /* 检测代码块边界（``` 开头的行切换状态） */
        if (line.startsWith(QStringLiteral("```"))) {
            in_code_block = !in_code_block;
            continue;
        }
        if (in_code_block) {
            continue;
        }

        QRegularExpressionMatch match = heading_re.match(line);
        if (match.hasMatch()) {
            OutlineItem item;
            item.level       = match.captured(1).length();
            item.title       = match.captured(2).trimmed();
            item.line_number = i;
            items.append(item);
        }
    }

    return items;
}
