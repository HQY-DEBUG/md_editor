/*
 * 文件 : render_worker.cpp
 * 描述 : Markdown 渲染工作对象实现
 * 版本 : v1.0
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   dev      26/03/19   创建文件
 */
#include "render_worker.h"

#include <QTextDocument>

RenderWorker::RenderWorker(QObject *parent)
    : QObject(parent)
{
}

void RenderWorker::render(const QString &markdown)
{
    if (markdown.isEmpty()) {
        emit renderFinished(QString());
        return;
    }

    /*
     * 使用 Qt 5.14+ 内置的 Markdown 解析器。
     * QTextDocument 在此处于工作线程栈上局部创建，不跨线程共享，线程安全。
     * setMarkdown / toHtml 均为纯数据处理，不涉及绘制操作。
     */
    QTextDocument doc;
    doc.setMarkdown(markdown, QTextDocument::MarkdownDialectGitHub);
    emit renderFinished(doc.toHtml());
}
