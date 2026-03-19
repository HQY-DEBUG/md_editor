/*
 * 文件 : file_worker.cpp
 * 描述 : 文件读写工作对象实现
 * 版本 : v1.0
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
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
    const QString content = stream.readAll();
    file.close();

    emit fileLoaded(content, path);
}

void FileWorker::save_file(const QString &path, const QString &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(QStringLiteral("无法保存文件：") + file.errorString());
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << content;
    file.close();

    emit fileSaved(path);
}

void FileWorker::export_html(const QString &path, const QString &html_content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(QStringLiteral("无法导出 HTML：") + file.errorString());
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << html_content;
    file.close();

    emit exportFinished(path);
}
