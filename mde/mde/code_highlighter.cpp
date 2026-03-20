/*
 * 文件 : code_highlighter.cpp
 * 描述 : 代码块语言语法高亮规则库实现
 * 版本 : v1.1
 * 日期 : 2026-03-19
 *
 * 修改记录（最新在前）:
 *  ver  who      date       modification
 * ----- -------  ---------- ---------------------------------
 * 1.1   dev      26/03/19   新增 Verilog/SystemVerilog 语法高亮
 * 1.0   dev      26/03/19   创建文件
 */
#include "code_highlighter.h"

#include <QFont>
#include <QColor>
#include <QHash>

// ============================================================
//  格式辅助（文件内部使用）
// ============================================================

namespace {

QTextCharFormat fmt_keyword()
{
    QTextCharFormat f;
    f.setForeground(QColor(0x00, 0x00, 0xCC));
    f.setFontWeight(QFont::Bold);
    return f;
}

QTextCharFormat fmt_string()
{
    QTextCharFormat f;
    f.setForeground(QColor(0x06, 0x7D, 0x17));
    return f;
}

QTextCharFormat fmt_comment()
{
    QTextCharFormat f;
    f.setForeground(QColor(0x60, 0x60, 0x60));
    f.setFontItalic(true);
    return f;
}

QTextCharFormat fmt_number()
{
    QTextCharFormat f;
    f.setForeground(QColor(0x09, 0x87, 0x65));
    return f;
}

QTextCharFormat fmt_preproc()
{
    QTextCharFormat f;
    f.setForeground(QColor(0x7B, 0x00, 0xD4));
    return f;
}

QTextCharFormat fmt_special()  /* 变量、注解、装饰器等 */
{
    QTextCharFormat f;
    f.setForeground(QColor(0xAF, 0x00, 0xAF));
    return f;
}

CodeRule make_rule(const QString &pattern, const QTextCharFormat &fmt,
                   QRegularExpression::PatternOptions opts =
                       QRegularExpression::NoPatternOption)
{
    return { QRegularExpression(pattern, opts), fmt };
}

} // namespace

// ============================================================
//  公开接口
// ============================================================

QString CodeHighlighter::normalize(const QString &raw)
{
    if (raw == "c"    || raw == "cpp"  || raw == "c++"  || raw == "cxx"  ||
        raw == "cc"   || raw == "h"    || raw == "hpp"  || raw == "hxx")
        return QStringLiteral("cpp");

    if (raw == "python" || raw == "py")
        return QStringLiteral("python");

    if (raw == "javascript" || raw == "js"  || raw == "jsx" ||
        raw == "typescript" || raw == "ts"  || raw == "tsx")
        return QStringLiteral("js");

    if (raw == "java")
        return QStringLiteral("java");

    if (raw == "bash"   || raw == "sh"   || raw == "shell" ||
        raw == "zsh"    || raw == "fish")
        return QStringLiteral("bash");

    if (raw == "go" || raw == "golang")
        return QStringLiteral("go");

    if (raw == "rust" || raw == "rs")
        return QStringLiteral("rust");

    if (raw == "sql")
        return QStringLiteral("sql");

    if (raw == "json")
        return QStringLiteral("json");

    if (raw == "verilog" || raw == "v" || raw == "sv" || raw == "systemverilog")
        return QStringLiteral("verilog");

    return raw;
}

const QVector<CodeRule> &CodeHighlighter::rules_for(const QString &lang)
{
    /* 静态缓存：每种语言的规则只构建一次 */
    static QHash<QString, QVector<CodeRule>> cache;

    auto it = cache.find(lang);
    if (it != cache.end())
        return it.value();

    QVector<CodeRule> rules;
    if      (lang == "cpp")    rules = build_cpp_rules();
    else if (lang == "python") rules = build_python_rules();
    else if (lang == "js")     rules = build_js_rules();
    else if (lang == "java")   rules = build_java_rules();
    else if (lang == "bash")   rules = build_bash_rules();
    else if (lang == "go")     rules = build_go_rules();
    else if (lang == "rust")   rules = build_rust_rules();
    else if (lang == "sql")    rules = build_sql_rules();
    else if (lang == "json")    rules = build_json_rules();
    else if (lang == "verilog") rules = build_verilog_rules();

    cache.insert(lang, rules);
    return cache[lang];
}

// ============================================================
//  各语言规则构建
//  规则按优先级从低到高排列——后面的规则会覆盖前面的格式
// ============================================================

QVector<CodeRule> CodeHighlighter::build_cpp_rules()
{
    QVector<CodeRule> r;

    /* 关键字 */
    r.append(make_rule(
        QStringLiteral("\\b(alignas|alignof|and|and_eq|asm|auto|bitand|bitor|bool|break|"
                        "case|catch|char|char8_t|char16_t|char32_t|class|compl|concept|"
                        "const|consteval|constexpr|constinit|const_cast|continue|co_await|"
                        "co_return|co_yield|decltype|default|delete|do|double|dynamic_cast|"
                        "else|enum|explicit|export|extern|false|float|for|friend|goto|if|"
                        "inline|int|long|mutable|namespace|new|noexcept|not|not_eq|nullptr|"
                        "operator|or|or_eq|override|private|protected|public|register|"
                        "reinterpret_cast|requires|return|short|signed|sizeof|static|"
                        "static_assert|static_cast|struct|switch|template|this|thread_local|"
                        "throw|true|try|typedef|typeid|typename|union|unsigned|using|virtual|"
                        "void|volatile|wchar_t|while|xor|xor_eq|final|nullptr_t)\\b"),
        fmt_keyword()
    ));

    /* 预处理指令（整行着色） */
    r.append(make_rule(
        QStringLiteral("^\\s*#\\s*(include|define|undef|ifdef|ifndef|if|elif|else|endif|"
                        "pragma|error|warning|line)\\b.*$"),
        fmt_preproc()
    ));

    /* 数值字面量：十六进制 / 八进制 / 浮点 / 整数 */
    r.append(make_rule(
        QStringLiteral("\\b(0[xX][0-9a-fA-F']+[uUlL]*"
                        "|0[bB][01']+[uUlL]*"
                        "|0[0-7']+[uUlL]*"
                        "|\\d[\\d']*\\.?[\\d']*([eE][+-]?[\\d']+)?[fFlLuU]*)\\b"),
        fmt_number()
    ));

    /* 字符串字面量（不跨行） */
    r.append(make_rule(QStringLiteral("\"[^\"\n]*\""), fmt_string()));

    /* 字符字面量 */
    r.append(make_rule(QStringLiteral("'[^'\n]+'"), fmt_string()));

    /* 行注释（最高优先级，覆盖上方所有格式） */
    r.append(make_rule(QStringLiteral("//[^\n]*"), fmt_comment()));

    /* 同行块注释 */
    r.append(make_rule(QStringLiteral("/\\*.*\\*/"), fmt_comment()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_python_rules()
{
    QVector<CodeRule> r;

    /* 关键字 */
    r.append(make_rule(
        QStringLiteral("\\b(False|None|True|and|as|assert|async|await|break|class|"
                        "continue|def|del|elif|else|except|finally|for|from|global|if|"
                        "import|in|is|lambda|nonlocal|not|or|pass|raise|return|try|"
                        "while|with|yield)\\b"),
        fmt_keyword()
    ));

    /* 内置函数 */
    r.append(make_rule(
        QStringLiteral("\\b(abs|all|any|bin|bool|breakpoint|bytearray|bytes|callable|"
                        "chr|classmethod|compile|complex|delattr|dict|dir|divmod|"
                        "enumerate|eval|exec|filter|float|format|frozenset|getattr|"
                        "globals|hasattr|hash|help|hex|id|input|int|isinstance|"
                        "issubclass|iter|len|list|locals|map|max|memoryview|min|"
                        "next|object|oct|open|ord|pow|print|property|range|repr|"
                        "reversed|round|set|setattr|slice|sorted|staticmethod|str|"
                        "sum|super|tuple|type|vars|zip)\\b"),
        fmt_special()
    ));

    /* 装饰器 */
    r.append(make_rule(QStringLiteral("@[A-Za-z_][A-Za-z0-9_.]*"), fmt_preproc()));

    /* 数值 */
    r.append(make_rule(
        QStringLiteral("\\b(0[xX][0-9a-fA-F_]+|0[bB][01_]+|0[oO][0-7_]+|"
                        "\\d[\\d_]*\\.?[\\d_]*([eE][+-]?[\\d_]+)?[jJ]?)\\b"),
        fmt_number()
    ));

    /* 字符串（双引号 / 单引号，不跨行） */
    r.append(make_rule(QStringLiteral("\"[^\"\n]*\""), fmt_string()));
    r.append(make_rule(QStringLiteral("'[^'\n]*'"),   fmt_string()));

    /* 注释 */
    r.append(make_rule(QStringLiteral("#[^\n]*"), fmt_comment()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_js_rules()
{
    QVector<CodeRule> r;

    /* 关键字（含 TypeScript 扩展） */
    r.append(make_rule(
        QStringLiteral("\\b(abstract|as|async|await|break|case|catch|class|const|"
                        "constructor|continue|debugger|declare|default|delete|do|else|"
                        "enum|export|extends|false|finally|for|from|function|if|"
                        "implements|import|in|instanceof|interface|let|namespace|new|"
                        "null|of|override|private|protected|public|readonly|return|"
                        "static|super|switch|this|throw|true|try|type|typeof|"
                        "undefined|var|void|while|with|yield)\\b"),
        fmt_keyword()
    ));

    /* 数值 */
    r.append(make_rule(
        QStringLiteral("\\b(0[xX][0-9a-fA-F_]+|0[bB][01_]+|0[oO][0-7_]+|"
                        "\\d[\\d_]*\\.?[\\d_]*([eE][+-]?[\\d_]+)?)\\b"),
        fmt_number()
    ));

    /* 模板字符串 `...`（不跨行） */
    r.append(make_rule(QStringLiteral("`[^`\n]*`"), fmt_string()));

    /* 字符串 */
    r.append(make_rule(QStringLiteral("\"[^\"\n]*\""), fmt_string()));
    r.append(make_rule(QStringLiteral("'[^'\n]*'"),   fmt_string()));

    /* 注释 */
    r.append(make_rule(QStringLiteral("//[^\n]*"),   fmt_comment()));
    r.append(make_rule(QStringLiteral("/\\*.*\\*/"), fmt_comment()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_java_rules()
{
    QVector<CodeRule> r;

    /* 关键字 */
    r.append(make_rule(
        QStringLiteral("\\b(abstract|assert|boolean|break|byte|case|catch|char|class|"
                        "const|continue|default|do|double|else|enum|extends|false|final|"
                        "finally|float|for|goto|if|implements|import|instanceof|int|"
                        "interface|long|native|new|null|package|private|protected|public|"
                        "record|return|sealed|short|static|strictfp|super|switch|"
                        "synchronized|this|throw|throws|transient|true|try|var|void|"
                        "volatile|while|yields)\\b"),
        fmt_keyword()
    ));

    /* 注解 */
    r.append(make_rule(QStringLiteral("@[A-Za-z_][A-Za-z0-9_]*"), fmt_preproc()));

    /* 数值 */
    r.append(make_rule(
        QStringLiteral("\\b(0[xX][0-9a-fA-F_]+[lL]?|0[bB][01_]+[lL]?|"
                        "\\d[\\d_]*\\.?[\\d_]*([eE][+-]?[\\d_]+)?[fFdDlL]?)\\b"),
        fmt_number()
    ));

    /* 字符串 / 字符 */
    r.append(make_rule(QStringLiteral("\"[^\"\n]*\""), fmt_string()));
    r.append(make_rule(QStringLiteral("'[^'\n]+'"),   fmt_string()));

    /* 注释 */
    r.append(make_rule(QStringLiteral("//[^\n]*"),   fmt_comment()));
    r.append(make_rule(QStringLiteral("/\\*.*\\*/"), fmt_comment()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_bash_rules()
{
    QVector<CodeRule> r;

    /* 关键字 */
    r.append(make_rule(
        QStringLiteral("\\b(if|then|else|elif|fi|for|while|until|do|done|case|esac|"
                        "in|select|function|return|local|declare|typeset|readonly|"
                        "break|continue|shift|exit|trap|unset|export)\\b"),
        fmt_keyword()
    ));

    /* 内置命令 */
    r.append(make_rule(
        QStringLiteral("\\b(echo|printf|read|source|alias|cd|ls|grep|sed|awk|"
                        "find|test|eval|exec|kill|wait|jobs|fg|bg|pwd|pushd|popd|"
                        "true|false|set|unset|getopts|help|type|which|command)\\b"),
        fmt_special()
    ));

    /* 变量引用 $VAR / ${VAR} / $1 */
    r.append(make_rule(QStringLiteral("\\$\\{?[A-Za-z_][A-Za-z0-9_]*\\}?|\\$[0-9#@*?!$]"),
                       fmt_preproc()));

    /* 数值 */
    r.append(make_rule(QStringLiteral("\\b\\d+\\b"), fmt_number()));

    /* 字符串 */
    r.append(make_rule(QStringLiteral("\"[^\"\n]*\""), fmt_string()));
    r.append(make_rule(QStringLiteral("'[^'\n]*'"),   fmt_string()));

    /* 注释 */
    r.append(make_rule(QStringLiteral("#[^\n]*"), fmt_comment()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_go_rules()
{
    QVector<CodeRule> r;

    /* 关键字 */
    r.append(make_rule(
        QStringLiteral("\\b(break|case|chan|const|continue|default|defer|else|"
                        "fallthrough|for|func|go|goto|if|import|interface|map|"
                        "package|range|return|select|struct|switch|type|var)\\b"),
        fmt_keyword()
    ));

    /* 内置函数 */
    r.append(make_rule(
        QStringLiteral("\\b(append|cap|close|complex|copy|delete|imag|len|make|"
                        "new|panic|print|println|real|recover)\\b"),
        fmt_special()
    ));

    /* 数值 */
    r.append(make_rule(
        QStringLiteral("\\b(0[xX][0-9a-fA-F_]+|0[bB][01_]+|0[oO][0-7_]+|"
                        "\\d[\\d_]*\\.?[\\d_]*([eE][+-]?[\\d_]+)?(i)?)\\b"),
        fmt_number()
    ));

    /* 原始字符串（反引号，不跨行近似处理） */
    r.append(make_rule(QStringLiteral("`[^`\n]*`"), fmt_string()));

    /* 字符串 / 字符 */
    r.append(make_rule(QStringLiteral("\"[^\"\n]*\""), fmt_string()));
    r.append(make_rule(QStringLiteral("'[^'\n]+'"),   fmt_string()));

    /* 注释 */
    r.append(make_rule(QStringLiteral("//[^\n]*"),   fmt_comment()));
    r.append(make_rule(QStringLiteral("/\\*.*\\*/"), fmt_comment()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_rust_rules()
{
    QVector<CodeRule> r;

    /* 基础类型 */
    r.append(make_rule(
        QStringLiteral("\\b(bool|char|f32|f64|i8|i16|i32|i64|i128|isize|"
                        "str|u8|u16|u32|u64|u128|usize)\\b"),
        fmt_special()
    ));

    /* 关键字 */
    r.append(make_rule(
        QStringLiteral("\\b(as|async|await|break|const|continue|crate|dyn|else|"
                        "enum|extern|false|fn|for|if|impl|in|let|loop|match|mod|"
                        "move|mut|pub|ref|return|self|Self|static|struct|super|"
                        "trait|true|type|unsafe|use|where|while|abstract|become|"
                        "box|do|final|macro|override|priv|try|typeof|union|"
                        "unsized|virtual|yield)\\b"),
        fmt_keyword()
    ));

    /* 宏调用 name! */
    r.append(make_rule(QStringLiteral("\\b[A-Za-z_][A-Za-z0-9_]*!(?=[^=])"), fmt_preproc()));

    /* 生命周期注解 'a */
    r.append(make_rule(QStringLiteral("'[a-z_][a-z0-9_]*\\b"), fmt_preproc()));

    /* 数值 */
    r.append(make_rule(
        QStringLiteral("\\b(0[xX][0-9a-fA-F_]+|0[bB][01_]+|0[oO][0-7_]+|"
                        "\\d[\\d_]*\\.?[\\d_]*([eE][+-]?[\\d_]+)?)(u8|u16|u32|u64|"
                        "u128|usize|i8|i16|i32|i64|i128|isize|f32|f64)?\\b"),
        fmt_number()
    ));

    /* 字符串 / 字节字符串 */
    r.append(make_rule(QStringLiteral("b?\"[^\"\n]*\""), fmt_string()));
    r.append(make_rule(QStringLiteral("b?'[^'\n]+'"),   fmt_string()));

    /* 注释 */
    r.append(make_rule(QStringLiteral("//[^\n]*"),   fmt_comment()));
    r.append(make_rule(QStringLiteral("/\\*.*\\*/"), fmt_comment()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_sql_rules()
{
    QVector<CodeRule> r;

    /* 关键字（大小写均支持） */
    r.append(make_rule(
        QStringLiteral("\\b(ADD|ALL|ALTER|AND|AS|ASC|BETWEEN|BY|CASE|CHECK|COLUMN|"
                        "CONSTRAINT|CREATE|CROSS|DATABASE|DEFAULT|DELETE|DESC|DISTINCT|"
                        "DROP|ELSE|END|EXISTS|FOREIGN|FROM|FULL|GROUP|HAVING|IN|INDEX|"
                        "INNER|INSERT|INTO|IS|JOIN|KEY|LEFT|LIKE|LIMIT|NOT|NULL|OFFSET|"
                        "ON|OR|ORDER|OUTER|PRIMARY|REFERENCES|RIGHT|ROWNUM|SELECT|SET|"
                        "TABLE|THEN|TOP|TRUNCATE|UNION|UNIQUE|UPDATE|VALUES|VIEW|"
                        "WHEN|WHERE|WITH)\\b"),
        fmt_keyword(),
        QRegularExpression::CaseInsensitiveOption
    ));

    /* 数值 */
    r.append(make_rule(QStringLiteral("\\b-?\\d+\\.?\\d*\\b"), fmt_number()));

    /* 字符串（单引号） */
    r.append(make_rule(QStringLiteral("'[^'\n]*'"), fmt_string()));

    // 注释（-- 行注释 和 /* */ 块注释）
    r.append(make_rule(QStringLiteral("--[^\n]*"),    fmt_comment()));
    r.append(make_rule(QStringLiteral("/\\*.*\\*/"),  fmt_comment()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_json_rules()
{
    QVector<CodeRule> r;

    /* 字符串值（先着绿，键名随后用紫色覆盖） */
    r.append(make_rule(QStringLiteral("\"[^\"\n]*\""), fmt_string()));

    /* 数值（含负数和小数） */
    r.append(make_rule(
        QStringLiteral("-?\\b\\d+\\.?\\d*([eE][+-]?\\d+)?\\b"),
        fmt_number()
    ));

    /* 布尔值和 null */
    r.append(make_rule(QStringLiteral("\\b(true|false|null)\\b"), fmt_keyword()));

    /* JSON 键（"key" 后跟 :，紫色覆盖绿色字符串格式） */
    r.append(make_rule(QStringLiteral("\"[^\"]*\"(?=\\s*:)"), fmt_special()));

    return r;
}

QVector<CodeRule> CodeHighlighter::build_verilog_rules()
{
    QVector<CodeRule> r;

    /* 关键字：模块结构 / 行为建模 / 时序 / SystemVerilog 扩展 */
    r.append(make_rule(
        QStringLiteral("\\b(always|always_comb|always_ff|always_latch|and|assign|"
                        "automatic|begin|buf|bufif0|bufif1|case|casex|casez|cell|"
                        "cmos|config|deassign|default|defparam|design|disable|edge|"
                        "else|end|endcase|endconfig|endfunction|endgenerate|"
                        "endmodule|endprimitive|endspecify|endtable|endtask|"
                        "event|for|force|forever|fork|function|generate|genvar|"
                        "highz0|highz1|if|ifnone|incdir|include|initial|inout|"
                        "input|instance|integer|join|large|liblist|library|"
                        "localparam|macromodule|medium|module|nand|negedge|nmos|"
                        "nor|noshowcancelled|not|notif0|notif1|or|output|parameter|"
                        "pmos|posedge|primitive|pull0|pull1|pulldown|pullup|"
                        "pulsestyle_ondetect|pulsestyle_onevent|rcmos|real|"
                        "realtime|reg|release|repeat|rnmos|rpmos|rtran|rtranif0|"
                        "rtranif1|scalared|showcancelled|signed|small|specify|"
                        "specparam|strong0|strong1|supply0|supply1|table|task|"
                        "time|tran|tranif0|tranif1|tri|tri0|tri1|triand|trior|"
                        "trireg|unsigned|use|uwire|vectored|wait|wand|weak0|"
                        "weak1|while|wire|wor|xnor|xor|"
                        /* SystemVerilog 扩展 */
                        "assert|assume|bit|break|byte|chandle|class|clocking|"
                        "constraint|context|continue|cover|covergroup|coverpoint|"
                        "cross|dist|do|endclass|endclocking|endgroup|endinterface|"
                        "endpackage|endprogram|endproperty|endsequence|enum|"
                        "expect|export|extends|extern|final|first_match|foreach|"
                        "forkjoin|iff|ignore_bins|illegal_bins|import|inside|"
                        "int|interface|intersect|join_any|join_none|let|local|"
                        "logic|longint|matches|modport|new|null|package|packed|"
                        "priority|program|property|protected|pure|rand|randc|"
                        "randcase|randsequence|ref|return|sequence|shortint|"
                        "shortreal|solve|static|string|struct|super|this|"
                        "throughout|timeprecision|timeunit|type|typedef|union|"
                        "unique|unique0|var|virtual|void|wait_order|wildcard|"
                        "with|within)\\b"),
        fmt_keyword()
    ));

    /* 编译器指令（`define、`include、`timescale 等） */
    r.append(make_rule(
        QStringLiteral("`(define|undef|ifdef|ifndef|elsif|else|endif|include|"
                        "timescale|default_nettype|resetall|unconnected_drive|"
                        "nounconnected_drive|celldefine|endcelldefine|"
                        "pragma|begin_keywords|end_keywords|line|"
                        "__FILE__|__LINE__)\\b"),
        fmt_preproc()
    ));

    /* 宏引用（`MACRO_NAME，非关键字指令） */
    r.append(make_rule(QStringLiteral("`[A-Za-z_][A-Za-z0-9_]*"), fmt_preproc()));

    /* 系统任务 / 系统函数（$display、$time 等） */
    r.append(make_rule(QStringLiteral("\\$[A-Za-z_][A-Za-z0-9_]*"), fmt_special()));

    /* 数值字面量（带位宽：4'b1010  8'hFF  16'd100  1'b0 等） */
    r.append(make_rule(
        QStringLiteral("\\b\\d*'[sS]?[bBoOdDhH][0-9a-fA-FxXzZ?_]+"),
        fmt_number()
    ));

    /* 普通十进制整数 */
    r.append(make_rule(QStringLiteral("\\b\\d+\\b"), fmt_number()));

    /* 字符串字面量 */
    r.append(make_rule(QStringLiteral("\"[^\"\n]*\""), fmt_string()));

    /* 行注释（最高优先级） */
    r.append(make_rule(QStringLiteral("//[^\n]*"), fmt_comment()));

    /* 同行块注释 */
    r.append(make_rule(QStringLiteral("/\\*.*\\*/"), fmt_comment()));

    return r;
}
