/*
 * 文件 : code_highlighter.h
 * 描述 : 代码块语言语法高亮规则库（静态工具类）
 * 版本 : v1.1
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.1   dev      26/03/19   新增 Verilog/SystemVerilog 语法高亮
 * 1.0   dev      26/03/19   创建文件，支持 C/C++、Python、JS/TS、Java、Bash、Go、Rust、SQL、JSON
 */
#pragma once

#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>
#include <QString>

/**
 * @brief 单条代码语法高亮规则
 */
struct CodeRule {
    QRegularExpression pattern; ///< 匹配模式
    QTextCharFormat    format;  ///< 匹配范围应用的文本格式
};

/**
 * @brief 代码块语法高亮规则库
 *
 * 纯静态工具类，按语言名返回单行高亮规则列表。
 * 内部使用静态缓存，每种语言的规则仅构建一次。
 * 不支持跨行注释/字符串（代码块内仅做单行匹配）。
 */
class CodeHighlighter {
public:
    CodeHighlighter() = delete;

    /**
     * @brief 根据语言标识符返回高亮规则列表（规则按优先级从低到高排列，后应用的覆盖先应用的）
     * @param lang 已规范化的语言名（如 "cpp"、"python"），未知语言返回空列表
     */
    static const QVector<CodeRule> &rules_for(const QString &lang);

    /**
     * @brief 将原始语言标识符规范化（别名映射到标准名）
     * @param raw 用户在代码围栏后填写的语言名（小写）
     * @return 标准语言名，未知则原样返回
     */
    static QString normalize(const QString &raw);

private:
    static QVector<CodeRule> build_cpp_rules();
    static QVector<CodeRule> build_python_rules();
    static QVector<CodeRule> build_js_rules();
    static QVector<CodeRule> build_java_rules();
    static QVector<CodeRule> build_bash_rules();
    static QVector<CodeRule> build_go_rules();
    static QVector<CodeRule> build_rust_rules();
    static QVector<CodeRule> build_sql_rules();
    static QVector<CodeRule> build_json_rules();
    static QVector<CodeRule> build_verilog_rules();
};
