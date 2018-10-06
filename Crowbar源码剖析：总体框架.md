# Crowbar源码剖析：总体框架

## Introduction

Crowbar是《自制编程语言》一书中作者自己构思的无类型语言。本书一边向读者讲述编程语言的基本要素，一边讲解具体的实现方法，可作为编译技术的入门材料。至于书中另一门叫做Diksam的静态类型语言，等我学完了再来写。

## Layer-0

不管什么语言，本质上就是一个文本解析程序，只不过在解析的同时配上了动作的执行，此所谓词法+语法+语义。若没有特殊要求，制作自己的语言并非难事。要分析词法，我们有Lex；要分析语法，我们有Yacc；于是我们的任务只剩下编写语义了——给你一个解析好的语句，你究竟要做什么事情？

既然是C程序，那就从main函数看起吧。

### main()

读取命令行指定的代码文件，然后初始化一个“解释器”，用它编译、执行代码，最后回收解释器。真简单！

```c
fp = fopen(argv[1], "r")
interpreter = CRB_create_interpreter()
CRB_compile(interpreter, fp)
CRB_interpret(interpreter)
CRB_dispose_interpreter(interpreter)
MEM_dump_blocks(stdout)
```

## Layer-1

### 简介

本层让大家看看main函数的每一步大概做了些什么事情。

- CRB_create_interpreter：创建解释器
- CRB_compile：用Lex和Yacc生成的分析器编译源码，得到很多“东西”，填充到解释器中
- CRB_interpret：执行解释器里的“东西”
- CRB_dispose_interpreter：清除内存等收尾工作
- MEM_dump_blocks：debug用

### CRB_create_interpreter()

啥是解释器啊？其实就是个数据结构，存储着程序解析的结果。

```c
struct CRB_Interpreter_tag {
    MEM_Storage         interpreter_storage;
    MEM_Storage         execute_storage;
    Variable            *variable;
    FunctionDefinition  *function_list;
    StatementList       *statement_list;
    int                 current_line_number;
    Stack               stack;
    Heap                heap;
    CRB_LocalEnvironment    *top_environment;
};
```

- interpreter_storage / execute_storage是用来控制存储空间的（后面说）

- variable / function_list / statement_list都是链表，存储解析得来的“东西”（后面说）

- current_line_number记录当前解析到了源代码的哪一行，方便报错时给出具体位置

- stack / heap是运行时分配的存储空间，比如给数组用的（后面说）

> __不知道top_environment是什么！__

```c
CRB_Interpreter *CRB_create_interpreter(void)
{
    MEM_Storage storage;
    CRB_Interpreter *interpreter;

    storage = MEM_open_storage(0);
    interpreter = MEM_storage_malloc(storage, sizeof(struct CRB_Interpreter_tag));
    interpreter->interpreter_storage = storage;
    interpreter->execute_storage = NULL;
    interpreter->variable = NULL;
    interpreter->function_list = NULL;
    interpreter->statement_list = NULL;
    interpreter->current_line_number = 1;
    interpreter->stack.stack_alloc_size = 0;
    interpreter->stack.stack_pointer = 0;
    interpreter->stack.stack = MEM_malloc(sizeof(CRB_Value) * STACK_ALLOC_SIZE);
    interpreter->heap.current_heap_size = 0;
    interpreter->heap.current_threshold = HEAP_THRESHOLD_SIZE;
    interpreter->heap.header = NULL;
    interpreter->top_environment = NULL;

    crb_set_current_interpreter(interpreter);
    add_native_functions(interpreter);

    return interpreter;
}
```

本函数的执行过程如下：

  - `interpreter`是一个`CRB_Interpreter`的指针（`CRB_Interpreter`等同于`CRB_Interpreter_tag`）
  - 开一片存储空间，为解释器留出位置
      - `storage = MEM_open_storage(0);`
      - `interpreter = MEM_storage_malloc(storage, sizeof(struct CRB_Interpreter_tag));`
- 初始化各种成员变量
  - `interpreter->interpreter_storage = storage;`
  - `interpreter->stack.stack = MEM_malloc(sizeof(CRB_Value) * STACK_ALLOC_SIZE);`
  - 其他成员均为平凡的初始值
- 设置另一个包的static变量，没什么特殊的：`crb_set_current_interpreter(interpreter)`
- “注册”原生函数（后面说）：`add_native_functions(interpreter)`

### CRB_compile

初步的数据结构有了，开始解析代码：

```c
void CRB_compile(CRB_Interpreter *interpreter, FILE *fp)
{
    extern int yyparse(void);
    extern FILE *yyin;
    crb_set_current_interpreter(interpreter);
    yyin = fp;
    if (yyparse()) {
        fprintf(stderr, "Error ! Error ! Error !\n");
        exit(1);
    }
    crb_reset_string_literal_buffer();
}
```

yyparse()是整个函数的主干，它由Lex和Yacc共同生成。简单来说，这个函数逐个字符读取源码，和Lex文件里写的词法规则进行匹配，每匹配一次就触发一些“动作”并可能返回一种“token”，此所谓“词法分析”。每次返回的token经过累积后，又会满足Yacc文件里写的语法规则（移进/归约），于是又会据此做一些“动作”，此所谓“语法分析”。整个文件读完（术语这个过程叫做“one pass”，一趟），代码就解析得差不多了，解释器里的数据结构也会被填满。

Lex和Yacc文件的具体内容，留到后面说。

### CRB_interpret

现在就要遍历解释器里的“东西”，真正地执行代码了。

```c
void CRB_interpret(CRB_Interpreter *interpreter)
{
    interpreter->execute_storage = MEM_open_storage(0);
    crb_add_std_fp(interpreter);
    crb_execute_statement_list(interpreter, NULL, interpreter->statement_list);
    crb_garbage_collect(interpreter);
}
```
第一行，开辟一个“存储器”，后面说。

> __第二行，暂时说不清楚__

第三行，执行解释器里的语句链表，它就是在上一步编译的过程中生成的“东西”之一。它只是遍历链表，对每条语句调用`execute_statement`，而后者又根据语句类型，再调用不同的处理函数`execute_X_statement`，其中X可以是expression, global, if, while, for, return, break, continue等具体语句种类。Layer-2将介绍具体的处理过程。

第四行，垃圾回收，后面说。

### CRB_dispose_interpreter

垃圾回收，内存释放等等。在进一步了解Crowbar内存管理机制之前，不好说细。其实我也没有完全搞懂，且写且珍惜。

### MEM_dump_blocks

debug用，打印各个内存块的状态，省略了。

## 后续

刚才我对代码树进行了广度优先遍历，介绍了最简要的框架。接下来我将适当采取深度优先遍历，分几篇文章把每个模块的实现讲透彻，也让自己铭记于心。

国庆即将结束，进度依然捉急。工作日到来后，学习时间将大幅缩水，但这系列我绝对不🐦。真的。

GTMD实习！

<!--

## Layer-2

### 列表

- MEM_open_storage：生成一个存储器对象
- MEM_storage_malloc：分配内存
- MEM_dispose_storage：回收内存
- crb_set_current_interpreter：设置静态变量st_current_interpreter的值
- crb_reset_string_literal_buffer：清空st_string_literal_buffer
- crb_add_std_fp：设置全局变量`STDIN, STDOUT, STDERR`
- crb_execute_statement_list：执行解释器里的“东西”
- add_native_functions：注册内置函数
- release_global_strings：释放全局字符串
- yyparse：用Lex和Yacc生成的分析器编译源码，得到很多“东西”，填充到解释器中

###yyparse - Lex

- INITIAL
  - 保留字与保留符号：例如`function, if, true, =+-*/`，返回token
  - 标识符（identifier）：调用`crb_create_identifier`，返回token
  - int和double：调用`crb_alloc_expression`，返回token
  - 双引号：调用`crb_open_string_literal`，进入STRING_LITERAL_STATE状态
  - 换行符：调用`increment_line_number`
  - \#：进入COMMENT状态
  - 其他字符：报错
- COMMENT
  - 换行符：调用`increment_line_number`并进入INITIAL状态
- STRING_LITERAL_STATE
  - 双引号：调用`crb_alloc_expression`，`crb_close_string_literal`，进入INITIAL状态，返回token
  - 其他字符都会调用`crb_add_string_literal`，累积字符串的内容

### yyparse - Yacc

- 太麻烦了，主要在调用create.c里的函数，往解释器里填东西（怎么填进去的？？？没看到给解释器赋值的语句啊）

### crb_execute_statement_list

- 遍历参数提供的链表`StatementList*`，调用`execute_statement`
- 后者根据语句的类型，再调用不同的处理函数`execute_X_statement`，其中X可以是expression, global, if, while, for, return, break, continue，否则调用`DBG_panic`

## Layer-3

-->