/*
 * 文件 : render_worker.h
 * 描述 : Markdown 渲染工作对象（运行于独立 QThread）
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
 * @brief Markdown 渲染工作对象
 *
 * 通过 moveToThread 在独立工作线程中运行。
 * 接收 Markdown 文本，转换为 HTML 后通过信号发回 GUI 线程展示。
 *
 * @note 使用 Qt::QueuedConnection 与 GUI 线程通信，禁止直接操作控件。
 */
class RenderWorker : public QObject {
    Q_OBJECT

public:
    explicit RenderWorker(QObject *parent = nullptr);

public slots:
    /**
     * @brief 将 Markdown 文本渲染为 HTML（在工作线程中执行）
     * @param markdown 原始 Markdown 内容
     */
    void render(const QString &markdown);

signals:
    /**
     * @brief 渲染完成，返回 HTML 字符串给 GUI 线程
     * @param html 生成的 HTML 内容
     */
    void renderFinished(const QString &html);

    /**
     * @brief 渲染过程中发生错误
     * @param msg 错误描述
     */
    void errorOccurred(const QString &msg);
};
