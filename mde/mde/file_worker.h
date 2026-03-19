/*
 * 文件 : file_worker.h
 * 描述 : 文件读写工作对象（运行于独立 QThread）
 * 版本 : v1.0
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.0   dev      26/03/19   创建文件
 */
#pragma once

#include <QObject>
#include <QString>

/**
 * @brief 文件读写工作对象
 *
 * 通过 moveToThread 在独立工作线程中运行，避免大文件 IO 阻塞 GUI 线程。
 * 所有槽均通过 Qt::QueuedConnection 从 GUI 线程调用。
 */
class FileWorker : public QObject {
    Q_OBJECT

public:
    explicit FileWorker(QObject *parent = nullptr);

public slots:
    /**
     * @brief 从磁盘加载文件（UTF-8 编码）
     * @param path 文件绝对路径
     */
    void load_file(const QString &path);

    /**
     * @brief 将内容写入磁盘（UTF-8 编码）
     * @param path    目标文件路径
     * @param content 要写入的文本内容
     */
    void save_file(const QString &path, const QString &content);

    /**
     * @brief 将 HTML 内容导出为 .html 文件
     * @param path         目标文件路径
     * @param html_content HTML 字符串
     */
    void export_html(const QString &path, const QString &html_content);

signals:
    /**
     * @brief 文件加载成功
     * @param content 文件内容
     * @param path    文件路径（供主窗口更新标题）
     */
    void fileLoaded(const QString &content, const QString &path);

    /**
     * @brief 文件保存成功
     * @param path 已保存的文件路径
     */
    void fileSaved(const QString &path);

    /**
     * @brief HTML 导出成功
     * @param path 导出文件路径
     */
    void exportFinished(const QString &path);

    /**
     * @brief 文件操作出错
     * @param msg 错误描述（供主窗口弹窗提示）
     */
    void errorOccurred(const QString &msg);
};
