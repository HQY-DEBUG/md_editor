/*
 * 文件 : file_worker.cpp
 * 描述 : 文件读写工作对象实现
 * 版本 : v1.1
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.1   dev      26/03/19   提取 write_text_file 辅助，移除冗余 file.close()（RAII）
 * 1.0   dev      26/03/19   创建文件
 */
#include "file_worker.h"

#include <QFile>
#include <QTextStream>
#include <QTextCodec>

FileWorker::FileWorker(QObject *parent)
    : QObject(parent)
{
}

void FileWorker::load_file(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(QStringLiteral("无法打开文件：") + file.errorString());
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    emit fileLoaded(stream.readAll(), path);
    /* file 析构时自动关闭（RAII） */
}

bool FileWorker::write_text_file(const QString &path, const QString &content,
                                  const QString &error_prefix)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(error_prefix + file.errorString());
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << content;
    return true;
    /* file 析构时自动关闭（RAII） */
}

void FileWorker::save_file(const QString &path, const QString &content)
{
    if (write_text_file(path, content, QStringLiteral("无法保存文件：")))
        emit fileSaved(path);
}

void FileWorker::export_html(const QString &path, const QString &html_content)
{
    if (write_text_file(path, html_content, QStringLiteral("无法导出 HTML：")))
        emit exportFinished(path);
}
