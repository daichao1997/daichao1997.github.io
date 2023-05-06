# Crowbar源码剖析：语法

## Introduction

[上回书说到](https://daichao1997.github.io/Crowbar源码剖析：词法.html)，Crowbar的词法分析器将具体的源码文件解析为一串抽象的**符号**，并伴随着一系列附属动作。接下来轮到Crowbar的语法分析器登场了——它是如何解读这串符号的？解读的同时又做了些什么事情呢？我们一起来看看。

本篇有非常多杂七杂八的代码，看的时候一定要把握住最大的共同点——**语法**和**数据结构**的高度一致。很多代码都是相似的，把握住**设计思想**才是最关键的。

## Crowbar-Yacc

本层介绍两个最重要的语法符号——语句（expression）和表达式（statement）。

### Crowbar-Yacc-statement

Crowbar的程序由一系列**语句**组成，有的是包含**关键字**的特殊语句，有的则是普通的**表达式**。

```c
statement /* 语句 */
        : expression SEMICOLON /* 表达式 + 分号 */
        {
          $$ = crb_create_expression_statement($1);
        }
        | global_statement
        | if_statement
        | while_statement
        | for_statement
        | return_statement
        | break_statement
        | continue_statement
        ;
```

这里规约时调用了某函数`crb_create_expression_statement`，看名字就知道是在生成一个语句。不难想到，Crowbar也用专门的结构体保存语句，也由类型、行号、内容这几个部分组成。随着语句类型的不同，联合体u也有不同的解读方式，但都是为了保留必要的信息。例如，联合体中没有出现break和continue，因为它们并不携带其他信息（除非以后允许它们携带标签）。

```c
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

于是，`crb_create_expression_statement`就创建了一个Statement结构体，填充了类型、行号和表达式的内容。

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
```

至于其他特殊语句，我们后面说。

### Crowbar-Yacc-expression

所谓表达式，就是用**运算符**关联**常量**或**标识符**而得到的组合。为了保证运算符的优先级，我们常会设计一个分层的语法结构，让最简单、最原始的表达式，按照语法上的优先次序，分步组合成复杂表达式。下面是一个示意结构，具体的后面说。

```
primary_expression
-> postfix_expression
-> unary_expression
-> multiplicative_expression
-> additive_expression
-> relational_expression
-> equality_expression
-> logical_and_expression
-> logical_or_expression
-> expression
```

表达式在互相归约时，常常调用`crb_create_XXX_expression`以生成Expression对象。[上回也有提到](https://daichao1997.github.io/Crowbar源码剖析：词法.html)Expression对象，但我们没有深究其结构。现在看来，和Statement也是差不多，存着类型、行号和内容。

```c
struct Expression_tag {
    ExpressionType type;
    int line_number;
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

## Crowbar-Yacc-About_Statement

本层介绍其他语句相关的符号，它们大多是由语句和表达式引出的。介绍有详有略，请大家摸清规律、搞懂套路。

- global_statement
  - identifier_list
- if_statement
  - elsif_list
  - elsif
  - block
  - statement_list
- XX_statement
  - expression_opt

### Crowbar-Yacc-global_statement

本语句用于引用全局变量，例如`global v1, v2, v3;`。遇到这种语句时，`v1, v2, v3`先归约为`identifier_list`，然后整个语句才归约为`global_statement`。

```c
global_statement
        : GLOBAL_T identifier_list SEMICOLON
        {
            $$ = crb_create_global_statement($2);
        }
        ;
```

下面这个函数与`crb_create_expression_statement`类似。还记得Statement结构体里的`GlobalStatement global_s`吗？它就是用来存储global语句的信息的。

```c
Statement *crb_create_global_statement(IdentifierList *identifier_list)
{
    Statement *st = alloc_statement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;
    return st;
}
typedef struct {
    IdentifierList      *identifier_list;
} GlobalStatement;
```

### Crowbar-Yacc-global_statement-identifier_list

`global_statement`引出了`identifier_list`。用下面两个函数创建链表或增长链表`IdentifierList`，一堆标识符名字组成的链表。

```c
identifier_list
        : IDENTIFIER
        {
            $$ = crb_create_global_identifier($1);
        }
        | identifier_list COMMA IDENTIFIER
        {
            $$ = crb_chain_identifier($1, $3);
        }
        ;
IdentifierList *crb_chain_identifier(IdentifierList *list, char *identifier)
{
    IdentifierList *pos;
    for (pos = list; pos->next; pos = pos->next);
    pos->next = crb_create_global_identifier(identifier);
    return list;
}
IdentifierList *crb_create_global_identifier(char *identifier)
{
    IdentifierList *i_list = crb_malloc(sizeof(IdentifierList));
    i_list->name = identifier;
    i_list->next = NULL;
    return i_list;
}
typedef struct IdentifierList_tag {
    char        *name;
    struct IdentifierList_tag   *next;
} IdentifierList;
```

### Crowbar-Yacc-if_statement

本语句大家都很熟悉，四种情况分别对应elsif/else的有无，都调用`crb_create_if_statement`。

```c
// 语法规则
if_statement
        : IF LP expression RP block
        {
            $$ = crb_create_if_statement($3, $5, NULL, NULL);
        }
        | IF LP expression RP block ELSE block
        {
            $$ = crb_create_if_statement($3, $5, NULL, $7);
        }
        | IF LP expression RP block elsif_list
        {
            $$ = crb_create_if_statement($3, $5, $6, NULL);
        }
        | IF LP expression RP block elsif_list ELSE block
        {
            $$ = crb_create_if_statement($3, $5, $6, $8);
        }
        ;
// 统一动作
Statement *crb_create_if_statement(Expression *condition, Block *then_block, Elsif *elsif_list, Block *else_block) {
    Statement *st = alloc_statement(IF_STATEMENT);
    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elsif_list = elsif_list;
    st->u.if_s.else_block = else_block;
    return st;
}
// 和语法一致的数据结构
typedef struct {
    Expression  *condition;
    Block       *then_block;
    Elsif       *elsif_list;
    Block       *else_block;
} IfStatement;
```

### Crowbar-Yacc-if_statement-elsif_list

elsif_list就是一堆elsif，是if_statement的一部分。组织链表的方式与前面标识符类似。

```c
elsif_list
        : elsif
        | elsif_list elsif
        {
            $$ = crb_chain_elsif_list($1, $2);
        }
        ;
Elsif *crb_chain_elsif_list(Elsif *list, Elsif *add) {
    Elsif *pos;
    for (pos = list; pos->next; pos = pos->next);
    pos->next = add;
    return list;
}
```

### Crowbar-Yacc-if_statement-elsif_list-elsif

elsif跟if相似。

```c
elsif
        : ELSIF LP expression RP block
        {
            $$ = crb_create_elsif($3, $5);
        }
        ;
Elsif *crb_create_elsif(Expression *expr, Block *block) {
    Elsif *ei = crb_malloc(sizeof(Elsif));
    ei->condition = expr;
    ei->block = block;
    ei->next = NULL;
    return ei;
}
typedef struct Elsif_tag {
    Expression  *condition;
    Block       *block;
    struct Elsif_tag    *next;
} Elsif;
```

### Crowbar-Yacc-if_statement-block

所谓block就是用{}包含的代码块，里面包含的是一堆语句，于是用`crb_create_block`生成一个Block结构体，向其中填入块中的语句链表。

```c
block
        : LC statement_list RC
        {
            $$ = crb_create_block($2);
        }
        | LC RC
        {
            $$ = crb_create_block(NULL);
        }
        ;
Block *crb_create_block(StatementList *statement_list) {
    Block *block = crb_malloc(sizeof(Block));
    block->statement_list = statement_list;
    return block;
}
typedef struct {
    StatementList       *statement_list;
} Block;
```

### Crowbar-Yacc-if_statement-block-statement_list

而statement_list就是一堆语句，用链表组织，由`crb_create_statement_list`生成。

```c
statement_list
        : statement
        {
            $$ = crb_create_statement_list($1);
        }
        | statement_list statement
        {
            $$ = crb_chain_statement_list($1, $2);
        }
        ;
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
typedef struct StatementList_tag {
    Statement   *statement;
    struct StatementList_tag    *next;
} StatementList;
```

### Crowbar-Yacc-XX_statement

现在不用我说，大家也能猜到下面这些语句做了什么，不一个个贴了。

```c
while_statement
        : WHILE LP expression RP block
        {
            $$ = crb_create_while_statement($3, $5);
        }
        ;
for_statement
        : FOR LP expression_opt SEMICOLON expression_opt SEMICOLON
          expression_opt RP block
        {
            $$ = crb_create_for_statement($3, $5, $7, $9);
        }
        ;
return_statement
        : RETURN_T expression_opt SEMICOLON
        {
            $$ = crb_create_return_statement($2);
        }
        ;
break_statement
        : BREAK SEMICOLON
        {
            $$ = crb_create_break_statement();
        }
        ;
continue_statement
        : CONTINUE SEMICOLON
        {
            $$ = crb_create_continue_statement();
        }
        ;
```

### Crowbar-Yacc-XX_statement-expression_opt

可有可无的表达式。

```c
expression_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | expression
        ;
```
## Crowbar-Yacc-About_Expression

本层按结合的优先级，介绍其他表达式相关的符号。介绍有详有略，请大家摸清规律、搞懂套路。

### Crowbar-Yacc-primary_expression

最底层的表达式，`primary_expression`，我理解为“原始表达式”。各种类型的字面常量、标识符都能归约到此。此外，函数调用、用括号包住的表达式，也属于单元表达式。它们的运算优先级是最高的，或者说根本不涉及运算。

```c
primary_expression
        : IDENTIFIER LP argument_list RP /* foo(arg1, arg2) */
        {
            $$ = crb_create_function_call_expression($1, $3);
        }
        | IDENTIFIER LP RP /* foo() */
        {
            $$ = crb_create_function_call_expression($1, NULL);
        }
        | LP expression RP /* (foo+1) */
        {
            $$ = $2;
        }
        /* 标识符和字面常量 */
        | IDENTIFIER
        {
            $$ = crb_create_identifier_expression($1);
        }
        | INT_LITERAL
        | DOUBLE_LITERAL
        | STRING_LITERAL
        | TRUE_T
        {
            $$ = crb_create_boolean_expression(CRB_TRUE);
        }
        | FALSE_T
        {
            $$ = crb_create_boolean_expression(CRB_FALSE);
        }
        | NULL_T
        {
            $$ = crb_create_null_expression();
        }
        | array_literal
        ;
```

### Crowbar-Yacc-postfix_expression

下一级是`postfix_expression`，我理解为“试着做后缀运算的表达式”。说“试着做”是因为该表达式可能不含后缀运算，但如果有，则一定比其他运算更优先。

```c
postfix_expression
        : primary_expression /* 没有后缀运算，直接规约 */
        | postfix_expression LB expression RB /* foo[1] */
        {
            $$ = crb_create_index_expression($1, $3);
        }
        | postfix_expression DOT IDENTIFIER LP argument_list RP /* foo.method(arg) */
        {
            $$ = crb_create_method_call_expression($1, $3, $5);
        }
        | postfix_expression DOT IDENTIFIER LP RP /* foo.method() */
        {
            $$ = crb_create_method_call_expression($1, $3, NULL);
        }
        | postfix_expression INCREMENT /* foo++ */
        {
            $$ = crb_create_incdec_expression($1, INCREMENT_EXPRESSION);
        }
        | postfix_expression DECREMENT /* foo-- */
        {
            $$ = crb_create_incdec_expression($1, DECREMENT_EXPRESSION);
        }
        ;
```

### Crowbar-Yacc-XX_expression

剩下的这一大堆语法从上往下看，就能发现不过是分别尝试了：
- 单目运算（取负）
- 乘法、除法、取模
- 加法、减法
- 大于、大于等于、小于、小于等于
- 等于、不等于
- 逻辑与
- 逻辑或

于是它们的优先级也从高到低。

```c
unary_expression
        : postfix_expression
        | SUB unary_expression
        {
            $$ = crb_create_minus_expression($2);
        }
        ;
multiplicative_expression
        : unary_expression
        | multiplicative_expression MUL unary_expression
        {
            $$ = crb_create_binary_expression(MUL_EXPRESSION, $1, $3);
        }
        | multiplicative_expression DIV unary_expression
        {
            $$ = crb_create_binary_expression(DIV_EXPRESSION, $1, $3);
        }
        | multiplicative_expression MOD unary_expression
        {
            $$ = crb_create_binary_expression(MOD_EXPRESSION, $1, $3);
        }
        ;
additive_expression
        : multiplicative_expression
        | additive_expression ADD multiplicative_expression
        {
            $$ = crb_create_binary_expression(ADD_EXPRESSION, $1, $3);
        }
        | additive_expression SUB multiplicative_expression
        {
            $$ = crb_create_binary_expression(SUB_EXPRESSION, $1, $3);
        }
        ;
relational_expression
        : additive_expression
        | relational_expression GT additive_expression
        {
            $$ = crb_create_binary_expression(GT_EXPRESSION, $1, $3);
        }
        | relational_expression GE additive_expression
        {
            $$ = crb_create_binary_expression(GE_EXPRESSION, $1, $3);
        }
        | relational_expression LT additive_expression
        {
            $$ = crb_create_binary_expression(LT_EXPRESSION, $1, $3);
        }
        | relational_expression LE additive_expression
        {
            $$ = crb_create_binary_expression(LE_EXPRESSION, $1, $3);
        }
        ;
equality_expression
        : relational_expression
        | equality_expression EQ relational_expression
        {
            $$ = crb_create_binary_expression(EQ_EXPRESSION, $1, $3);
        }
        | equality_expression NE relational_expression
        {
            $$ = crb_create_binary_expression(NE_EXPRESSION, $1, $3);
        }
        ;
logical_and_expression
        : equality_expression
        | logical_and_expression LOGICAL_AND equality_expression
        {
            $$ = crb_create_binary_expression(LOGICAL_AND_EXPRESSION, $1, $3);
        }
        ;
logical_or_expression
        : logical_and_expression
        | logical_or_expression LOGICAL_OR logical_and_expression
        {
            $$ = crb_create_binary_expression(LOGICAL_OR_EXPRESSION, $1, $3);
        }
        ;
```

等到其他运算符结合完了，再来赋值。

```c
expression
        : logical_or_expression
        | postfix_expression ASSIGN expression
        {
            $$ = crb_create_assign_expression($1, $3);
        }
        ;
```

## Crowbar-Yacc-About_Expression-动作

本层主要介绍上面的表达式归约时的具体动作。

### Crowbar-Yacc-About_Expression-函数调用

包含**函数名**，**参数列表**。这也体现在了数据结构里。

```c
Expression *crb_create_function_call_expression(char *func_name, ArgumentList *argument) {
    Expression *exp = crb_alloc_expression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.identifier = func_name;
    exp->u.function_call_expression.argument = argument;
    return exp;
}
typedef struct {
    char                *identifier;
    ArgumentList        *argument;
} FunctionCallExpression;
```

### Crowbar-Yacc-About_Expression-函数调用-参数列表

参数列表就是由**逗号**分隔的**表达式**，依旧用链表组织，管理方式同前。

```c
argument_list
        : expression
        {
            $$ = crb_create_argument_list($1);
        }
        | argument_list COMMA expression
        {
            $$ = crb_chain_argument_list($1, $3);
        }
        ;
ArgumentList *crb_create_argument_list(Expression *expression) {
    ArgumentList *al = crb_malloc(sizeof(ArgumentList));
    al->expression = expression;
    al->next = NULL;
    return al;
}
ArgumentList *crb_chain_argument_list(ArgumentList *list, Expression *expr) {
    ArgumentList *pos;
    for (pos = list; pos->next; pos = pos->next);
    pos->next = crb_create_argument_list(expr);
    return list;
}
typedef struct ArgumentList_tag {
    Expression *expression;
    struct ArgumentList_tag *next;
} ArgumentList;
```

### Crowbar-Yacc-About_Expression-标识符

标识符只是字符串。

```c
Expression *crb_create_identifier_expression(char *identifier) {
    Expression *exp = crb_alloc_expression(IDENTIFIER_EXPRESSION);
    exp->u.identifier = identifier;
    return exp;
}
```

### Crowbar-Yacc-About_Expression-布尔变量

布尔变量是自己定义的，虽然直接用bool也没问题，但此方法可以让你设计更多种类的变量。

```c
Expression *crb_create_boolean_expression(CRB_Boolean value) {
    Expression *exp = crb_alloc_expression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;
    return exp;
}
typedef enum {
    CRB_FALSE = 0,
    CRB_TRUE = 1
} CRB_Boolean;
```

### Crowbar-Yacc-About_Expression-数组常量

用**花括号**括起来的**表达式列表**。

```c
array_literal
        : LC expression_list RC
        {
            $$ = crb_create_array_expression($2);
        }
        | LC expression_list COMMA RC
        {
            $$ = crb_create_array_expression($2);
        }
        ;
Expression *crb_create_array_expression(ExpressionList *list) {
    Expression *exp = crb_alloc_expression(ARRAY_EXPRESSION);
    exp->u.array_literal = list;
    return exp;
}
typedef struct ExpressionList_tag {
    Expression          *expression;
    struct ExpressionList_tag   *next;
} ExpressionList;
```

### Crowbar-Yacc-About_Expression-数组常量-表达式列表

表达式列表还是由**逗号**分隔的**表达式**，依旧用链表组织，管理方式同前。等等，这跟参数列表有啥区别？貌似唯一的区别是，表达式列表可以为空，而参数列表不行，真奇怪。

```c
expression_list
        : /* empty */
        {
            $$ = NULL;
        }
        | expression
        {
            $$ = crb_create_expression_list($1);
        }
        | expression_list COMMA expression
        {
            $$ = crb_chain_expression_list($1, $3);
        }
        ;
ExpressionList *crb_create_expression_list(Expression *expression) {
    ExpressionList *el = crb_malloc(sizeof(ExpressionList));
    el->expression = expression;
    el->next = NULL;
    return el;
}
ExpressionList *crb_chain_expression_list(ExpressionList *list, Expression *expr) {
    ExpressionList *pos;
    for (pos = list; pos->next; pos = pos->next);
    pos->next = crb_create_expression_list(expr);
    return list;
}
```

### Crowbar-Yacc-About_Expression-取数组元素

取数组元素，一要**数组**，二要**下标**。

```c
Expression *crb_create_index_expression(Expression *array, Expression *index) {
    Expression *exp = crb_alloc_expression(INDEX_EXPRESSION);
    exp->u.index_expression.array = array;
    exp->u.index_expression.index = index;
    return exp;
}
typedef struct {
    Expression  *array;
    Expression  *index;
} IndexExpression;
```

### Crowbar-Yacc-About_Expression-调用方法

调用方法，一要对象，二要方法名，三要参数列表，例如`foo.method(arg1, arg2)`。

```c
Expression *crb_create_method_call_expression(Expression *expression, char *method_name, ArgumentList *argument) {
    Expression *exp = crb_alloc_expression(METHOD_CALL_EXPRESSION);
    exp->u.method_call_expression.expression = expression;
    exp->u.method_call_expression.identifier = method_name;
    exp->u.method_call_expression.argument = argument;
    return exp;
}
typedef struct {
    Expression          *expression;
    char                *identifier;
    ArgumentList        *argument;
} MethodCallExpression;
```

### Crowbar-Yacc-About_Expression-自增自减

```c
Expression *crb_create_incdec_expression(Expression *operand, ExpressionType inc_or_dec) {
    Expression *exp = crb_alloc_expression(inc_or_dec);
    exp->u.inc_dec.operand = operand;
    return exp;
}
typedef struct {
    Expression  *operand;
} IncrementOrDecrement;
```

### Crowbar-Yacc-About_Expression-取负

负号如果用在字面常量上，得到的也是字面常量。为了减少语义分析的负担，这里先判断表达式的类型。如果是整数或浮点数，就求出其负值，再把该负值作为新的字面常量返回，而不是返回类型为`MINUS_EXPRESSION`的表达式。

`crb_eval_minus_expression`和`convert_value_to_expression`在语义分析中才会用到，因此也留到后面说。

```c
Expression *crb_create_minus_expression(Expression *operand) {
    if (operand->type == INT_EXPRESSION || operand->type == DOUBLE_EXPRESSION) {
        CRB_Value v = crb_eval_minus_expression(crb_get_current_interpreter(), NULL, operand);
        /* Notice! Overwriting operand expression. */
        *operand = convert_value_to_expression(&v);
        return operand;
    } else {
        Expression *exp = crb_alloc_expression(MINUS_EXPRESSION);
        exp->u.minus_expression = operand;
        return exp;
    }
}
```

### Crowbar-Yacc-About_Expression-二元运算

二元运算包括加法、减法、乘法、除法、取模、大于、大于等于、小于、小于等于、等于、不等于、逻辑与、逻辑或。但无论是哪种运算，都执行着相同的操作。

这里也做了优化：如果运算双方都是字面常量，那么返回的也是字面常量，而不是`BinaryExpression`。

```c
Expression *crb_create_binary_expression(ExpressionType operator, Expression *left, Expression *right) {
    if ((left->type == INT_EXPRESSION || left->type == DOUBLE_EXPRESSION)
        && (right->type == INT_EXPRESSION || right->type == DOUBLE_EXPRESSION)) {
        CRB_Value v = crb_eval_binary_expression(crb_get_current_interpreter(), NULL, operator, left, right);
        /* Overwriting left hand expression. */
        *left = convert_value_to_expression(&v);
        return left;
    } else {
        Expression *exp = crb_alloc_expression(operator);
        exp->u.binary_expression.left = left;
        exp->u.binary_expression.right = right;
        return exp;
    }
}
// 很明显，二元运算只需要左右操作数，上层的具体类型则由符号决定
typedef struct {
    Expression  *left;
    Expression  *right;
} BinaryExpression;
```

### Crowbar-Yacc-About_Expression-赋值

```c
Expression *crb_create_assign_expression(Expression *left, Expression *operand) {
    Expression *exp = crb_alloc_expression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.left = left;
    exp->u.assign_expression.operand = operand;
    return exp;
}
Expression *crb_alloc_expression(ExpressionType type) {
    Expression *exp = crb_malloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = crb_get_current_interpreter()->current_line_number;
    return exp;
}
typedef struct {
    Expression  *left;
    Expression  *operand;
} AssignExpression;
```

## 总结

词法和语法的部分应该是事无巨细地讲完了。看这么多杂七杂八的代码，一定要把握住最大的共同点——**语法**和**数据结构**的高度一致。语法是人定的，掌握了设计思想才能自由发挥，创造自己的语法。此外，一门合格的编程语言所需的基本要素，也能通过本篇文章看个大概——数值计算、函数调用、数组、语句块……

下次更新可能要很久之后了，应该会开始讲语义分析。