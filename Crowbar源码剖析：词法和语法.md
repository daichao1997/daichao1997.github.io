# Crowbar源码剖析：词法和语法

## 往期回顾

- [[2018-10-06] Crowbar源码剖析：总体框架](https://daichao1997.github.io/Crowbar源码剖析：总体框架.html)

## Introduction

[上回书说到](https://daichao1997.github.io/Crowbar源码剖析：总体框架.html)，Crowbar用了Lex/Yacc这对经典工具来生成词法/语法分析器。它们具体的编写规则我不想细说，请参考[这篇文章](https://segmentfault.com/a/1190000000396608)，但大致来讲，两者都是“一边匹配一边触发动作”。

本篇文章会集中讨论**匹配**触发了**哪些动作**，而不讲如何匹配、为什么匹配。

## Lex

### 保留字与运算符

直接返回对应类型的token。所谓token，就是**符号**，分为**非终结符**和**终结符**。

```c
<INITIAL>"function"     return FUNCTION;
<INITIAL>";"            return SEMICOLON;
<INITIAL>"++"           return INCREMENT;
// and more
```

### 标识符

匹配到标识符后，调用`crb_create_identifier`，malloc一个字符串。真简单！

```c
<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
    yylval.identifier = crb_create_identifier(yytext);
    return IDENTIFIER;
}

char *crb_create_identifier(char *str) {
    char *new_str;
    new_str = crb_malloc(strlen(str) + 1);
    strcpy(new_str, str);
    return new_str;
}
```

### 整数、浮点数

整数和浮点数都属于最简单的**表达式**，但任何表达式都要用专门的Expression结构体来存储，这样才能保证语法上的统一。因此我们调用`crb_alloc_expression`生成一个新Expression对象，然后把匹配到的字面值yytext记入其中。

```c
// 整数
<INITIAL>([1-9][0-9]*)|"0" {
    Expression  *expression = crb_alloc_expression(INT_EXPRESSION);
    sscanf(yytext, "%d", &expression->u.int_value);
    yylval.expression = expression;
    return INT_LITERAL;
}
// 浮点数
<INITIAL>[0-9]+\.[0-9]+ {
    Expression  *expression = crb_alloc_expression(DOUBLE_EXPRESSION);
    sscanf(yytext, "%lf", &expression->u.double_value);
    yylval.expression = expression;
    return DOUBLE_LITERAL;
}
```

`crb_alloc_expression`只是分配空间，完成简单的初始化。

```c
Expression *crb_alloc_expression(ExpressionType type) {
    Expression  *exp;

    exp = crb_malloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = crb_get_current_interpreter()->current_line_number;

    return exp;
}
```

Expression结构体记录着表达式的元信息，其中的联合体u随着type的不同，有着不同的解读方式，具体的后面说。

```c
struct Expression_tag {  // 等同于Expression
    ExpressionType type; // 枚举类型，记录表达式的类型
    int line_number;     // 所在行号
    // 根据需要，用不同的结构体记录不同类型的表达式
    union {
        CRB_Boolean             boolean_value;
        int                     int_value;
        double                  double_value;
        char                    *string_value;
        char                    *identifier;
        AssignExpression        assign_expression;
        BinaryExpression        binary_expression;
        Expression              *minus_expression;
        FunctionCallExpression  function_call_expression;
        MethodCallExpression    method_call_expression;
        ExpressionList          *array_literal;
        IndexExpression         index_expression;
        IncrementOrDecrement    inc_dec;
    } u;
};
```

### 字符串

识别到单个双引号后，先用`crb_open_string_literal`清零buffer，然后从"INITIAL"状态进入"STRING_LITERAL_STATE"状态，标志着字符串识别的开始。这些状态可以在Lex文件里自定义。

以下代码中的`SSLB`均代表`st_string_literal_buffer`（实在太长了，看不下去）

```c
<INITIAL>\"" { // 应为单个双引号，为显示美观而改
    crb_open_string_literal();
    BEGIN STRING_LITERAL_STATE;
}
void crb_open_string_literal(void) {
    SSLB_size = 0;
}
```

在字符串识别模式下，任何双引号之外的字符都会被简单地加入SSLB，也就是临时存放字符串的buffer。如果是换行符，还会增加一下行数。

将字符加入SSLB的具体方法是`crb_add_string_literal`，它除了往buffer里塞一个字符，还管理着buffer的大小。毕竟，字符串是不限长度的。

```c
<STRING_LITERAL_STATE>\n        {
    crb_add_string_literal('\n');
    increment_line_number();
}
<STRING_LITERAL_STATE>\\\""     crb_add_string_literal('"'); // 应为单个双引号，为显示美观而改
<STRING_LITERAL_STATE>\\n       crb_add_string_literal('\n');
<STRING_LITERAL_STATE>\\t       crb_add_string_literal('\t');
<STRING_LITERAL_STATE>\\\\      crb_add_string_literal('\\');
<STRING_LITERAL_STATE>.         crb_add_string_literal(yytext[0]);
void crb_add_string_literal(int letter) {
	// 只有buffer不够用了，才给它多分配点空间
    if (SSLB_size == SSLB_alloc_size) {
        SSLB_alloc_size += STRING_ALLOC_SIZE;
        SSLB = MEM_realloc(SSLB, SSLB_alloc_size);
    }
    SSLB[SSLB_size] = letter;
    SSLB_size++;
}
```

字符串结束后，还是像整数、浮点数那样处理。`crb_close_string_literal`只是把buffer的内容复制到新字符串里罢了。

```c
<STRING_LITERAL_STATE>\"" { // 应为单个双引号，为显示美观而改
    Expression *expression = crb_alloc_expression(STRING_EXPRESSION);
    expression->u.string_value = crb_close_string_literal();
    yylval.expression = expression;
    BEGIN INITIAL;
    return STRING_LITERAL;
}
char *crb_close_string_literal(void) {
    char *new_str;
    new_str = crb_malloc(st_string_literal_buffer_size + 1);
    memcpy(new_str, st_string_literal_buffer, st_string_literal_buffer_size);
    new_str[st_string_literal_buffer_size] = '\0';
    return new_str;
}
```

### 其他字符

- INITIAL状态下遇到井号\#会进入COMMENT状态，再遇到换行符才回到INITIAL，其他字符都无动作
- 换行符：行数加一
- 空格与Tab：无动作
- 其他：报错

### Lex总结

## Yacc