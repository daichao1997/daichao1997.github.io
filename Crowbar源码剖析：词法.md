# Crowbar源码剖析：词法

## Introduction

[上回书说到](https://daichao1997.github.io/Crowbar源码剖析：总体框架.html)，Crowbar用了Lex/Yacc这对经典工具来生成词法/语法分析器。它们具体的编写规则我不想细说，请参考[这篇文章](https://segmentfault.com/a/1190000000396608)，但大致来讲，两者都是“一边匹配一边触发动作”。

本篇文章会集中讨论**匹配**触发了**哪些动作**，而不讲如何匹配、为什么匹配。

## Layer-2-Lex

这部分讲述Lex如何对五花八门的字符作出不同反应，用`<X>exp{action}`表示“状态X下，与exp匹配时，执行action”。为了给Yacc传递符号的解析结果，双方约定了一种“通信方式”：

- 符号的**字面内容**自动进入了`char *yytext`
- 符号的**值**需要手动存入`yylval`（在Yacc里定义）
- 用`return`返回符号的**种类**（在Yacc里定义）

本来不想说Lex/Yacc的，但这种影响代码理解的规则，还是讲一下比较好。

### 保留字与运算符

匹配到保留字/运算符后，直接返回对应的token。

```c
<INITIAL>"function"     return FUNCTION;
<INITIAL>";"            return SEMICOLON;
<INITIAL>"++"           return INCREMENT;
// and more
```

### 标识符

匹配到标识符后，调用`crb_create_identifier`，把标识符装进一个新字符串。真简单！

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

整数和浮点数都属于最简单的**表达式**，但任何表达式都要用专门的Expression结构体来存储，这样才能保证语法上的统一。因此我们调用`crb_alloc_expression`生成一个新Expression对象，然后把匹配到的字面值（yytext）以正确的形式（int/double）记入其中。

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

### 字符串

识别到单个双引号后，先用`crb_open_string_literal`清空临时存放字符串的`st_string_literal_buffer`，然后从INITIAL状态进入STRING_LITERAL_STATE状态，标志着字符串识别的开始。

`#define SSLB st_string_literal_buffer //实在太长了，看不下去`

`#define \"" \" //Markdown不支持Lex语法，于是我想以C来显示。但如果下面只写一个双引号，后面的内容全都会被视为字符串，从而变成丑陋的红色，所以这里把双引号变成两个。`

```c
<INITIAL>\"" {
    crb_open_string_literal();
    BEGIN STRING_LITERAL_STATE;
}
void crb_open_string_literal(void) {
    SSLB_size = 0;
}
```

在字符串识别模式下，任何双引号之外的字符都会被简单地加入SSLB。如果是换行符，还会增加一下行数。

`crb_add_string_literal`专门用来将字符加入SSLB，它除了往buffer里塞一个字符，还管理着buffer的大小，毕竟字符串（理论上）是不限长度的。由此可见，如果语言的设计者懒得管理内存，那么用户可以有很多种方法把系统整崩溃。

```c
<STRING_LITERAL_STATE>\n        {
    crb_add_string_literal('\n');
    increment_line_number();
}
<STRING_LITERAL_STATE>\\\""     crb_add_string_literal('"');
<STRING_LITERAL_STATE>\\n       crb_add_string_literal('\n');
<STRING_LITERAL_STATE>\\t       crb_add_string_literal('\t');
<STRING_LITERAL_STATE>\\\\      crb_add_string_literal('\\');
<STRING_LITERAL_STATE>.         crb_add_string_literal(yytext[0]);
void crb_add_string_literal(int letter) {
	// 只有buffer满载了，才给它多分配点空间
    if (SSLB_size == SSLB_alloc_size) {
        SSLB_alloc_size += STRING_ALLOC_SIZE;
        SSLB = MEM_realloc(SSLB, SSLB_alloc_size);
    }
    SSLB[SSLB_size] = letter;
    SSLB_size++;
}
```

另一个双引号标志着字符串的结束，按表达式处理即可，如同整数、浮点数那样。`crb_close_string_literal`只是把buffer的内容复制到新字符串里罢了。

```c
<STRING_LITERAL_STATE>\"" {
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

- INITIAL状态下遇到\#会进入COMMENT状态，再遇到换行符才回到INITIAL，其他字符都无动作
- 换行符：行数加一
- 空格与Tab：无动作
- 其他：报错

## 总结

Lex将文本提取为一串抽象的符号。符号的**种类**供Yacc进行语法分析，蕴含的**值**则以正确的形式保留，而具体的**字面内容**已经不再重要。

其实Yacc的内容我已经写了一半，就在本文下方的注释里，但你们就是看不到！哈哈哈气不气！（mdzz）

注释掉是因为我明天赶火车，今天发不出来完整版。坐高铁的时候应该能完成Yacc剩下的部分，并争取明天发出来，给国庆节一个（相对）完整的交代——原本还奢望看完整本书呢。🚩这是我的flag，不🐦，真的。

说到离家回校，又有一堆感慨，但鉴于这篇是“技术”文章，我还是忍住吧。

GTMD实习！

<!--

## Layer-2-Yacc

如前所述，这里不说如何归约，只说归约后做什么。

### 语句块（block）

调用`crb_create_block`，生成一个Block结构体，填入块中的语句链表。

```c
Block *crb_create_block(StatementList *statement_list) {
    Block *block = crb_malloc(sizeof(Block));
    block->statement_list = statement_list;
    return block;
}

typedef struct {
    StatementList       *statement_list;
} Block;

typedef struct StatementList_tag {
    Statement   *statement;
    struct StatementList_tag    *next;
} StatementList;
```

### 语句链表（statement_list）

将语句加入已有的语句链表，没有就生成新的。

```c
StatementList *crb_chain_statement_list(StatementList *list, Statement *statement) {
    StatementList *pos;
    if (list == NULL) {
        return crb_create_statement_list(statement);
    }
    for (pos = list; pos->next; pos = pos->next); // 到链表尾部
    pos->next = crb_create_statement_list(statement); // 插入新元素
    return list;
}

StatementList *crb_create_statement_list(Statement *statement) {
    StatementList *sl = crb_malloc(sizeof(StatementList));
    sl->statement = statement;
    sl->next = NULL;
    return sl;
}
```

### 语句（statement）

如果是global/if/while/for/return/break/continue等语句，什么也不做。如果是表达式+分号，调用`crb_create_expression_statement`。

```c
Statement *crb_create_expression_statement(Expression *expression) {
    Statement *st = alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;
    return st;
}

static Statement *alloc_statement(StatementType type) {
    Statement *st = crb_malloc(sizeof(Statement));
    st->type = type;
    st->line_number = crb_get_current_interpreter()->current_line_number;
    return st;
}

struct Statement_tag {
    StatementType       type;
    int                 line_number;
    union {
        Expression      *expression_s;
        GlobalStatement global_s;
        IfStatement     if_s;
        WhileStatement  while_s;
        ForStatement    for_s;
        ReturnStatement return_s;
    } u;
};
```
-->