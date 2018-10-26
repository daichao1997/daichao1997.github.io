# C++查漏补缺

我已经不怎么记得“程序设计实习”的内容了。

## C与C++的（细微）区别

### C++支持，但C不支持

1. 面向对象
2. 重载、模板、继承、虚函数、友元函数
3. 语言层面上的异常处理
4. 引用
5. 输入输出流
6. new/delete/explicit/class等关键词
7. 结构体中定义成员函数
8. 结构体中定义静态变量
9. 直接在结构体中初始化成员变量

### C支持，但C++不支持

1. 调用尚未声明的函数
2. 将const的地址赋给non-const指针
3. 将void\*值直接赋给其他指针，malloc()返回的就是void\*
4. 不初始化const
5. `char *c = 333`

### 其他

1. C认为func()=func(...)，即func可以接受任意个数的参数。C++认为func()=func(void)，即func不能接受参数。因此在C中使用func(void)会显得你更专业、更严谨
2. C认为字面字符常量是整数，C++则认为是字符。`sizeof('a')`，前者得到4，后者得到1
3. C的结构体类型必须带上struct前缀。`struct T; T a;`是不合法的C代码，但C++认为没问题
4. `sizeof(1==1)`的C值为4，C++值为1，因为C++支持bool类型
5. 空结构体的sizeof值在C是0，在C++是1

## 引用和指针的区别

总地来说，引用不如指针灵活，但更安全。引用值必须

1. 声明时初始化
2. 不能为NULL
3. 不能改绑

这些限制避免了野指针的存在，但也导致开发者无法使用引用来编写链表等数据结构。Java的引用没有上面这些限制，因此Java可以直接抛弃指针的说法。

## "delete this"会发生什么？

首先，只有new出来的对象才能被delete，否则会产生undefined behavior；其次，这句话只能在非静态成员函数中使用，否则会产生编译错误；最后，正常地`delete this`之后，函数内将无法获取自己的成员变量。

## 虚函数

考虑一个简单的场景，基类指针b指向了派生类对象d，然后调用成员函数：

```cpp
class base {
public:
	virtual void print() {
		cout<< "print base class" <<endl;
	}
}; 
  
class derived : public base {
public:
    void print() {
		cout<< "print derived class" <<endl;
	}
};

int main(void) { //现学现卖
	derived d;
	base *b = &d;
	b->print();
}
```

### 规则

1. 必须定义在public部分
2. 不能是static函数
3. 不能是友元函数
4. 基类和派生类中的原型相同（这样才能覆盖）

### 工作原理

若一个类含有虚函数，那么每当它的一个对象被创建时，该类都会新增一个独特的VPTR成员变量，指向该类的VTABLE。VTABLE是每个类都有的静态成员，是一个函数指针的数组，分别指向该类的所有虚函数。

当指向派生类对象D的基类指针b调用成员函数f时：

- 若基类的f不是虚函数，则编译时就决定，要调用的是基类的f
- 若基类的f是虚函数，则运行时通过 b 所指向的 D 所指向的 VPTR 所指向的 VTABLE，找到要调用的函数（派生类的f），从而实现运行时的**多态**。

### 虚析构函数

若基类指针b指向派生类对象d，且基类没有虚析构函数，则`delete b`会产生未定义行为。加上虚析构函数后，结果才正常——先调用派生类的析构函数，依次向上。

### 纯虚函数

虚函数后面加上`=0`就是**纯虚函数**，例如`virtual int f() = 0;`。只要一个类有了纯虚函数作为成员，它就成了**抽象类**。

- 抽象类没有对象，但可以有指针，也可以有构造函数
- 抽象类的派生类还是抽象类，除非它实现了所有纯虚函数

## overload/override/hiding

- overload: 相同的函数名，不同的原型，称为**重载**___（不是过载，炉宗注意了）___
- override: 相同的函数名，相同的原型，用于**覆盖**父类虚函数
- hiding: 在内层作用域中定义和外层同名的变量，外层的变量就被**隐藏**了

## inline

inline只是**请求**编译器将函数代码直接插入调用处，从而省下函数调用的开销（此处略）。如果该函数具备这些特点，编译器可能不会进行真正的inlining：

1. 包含循环体
2. 包含静态变量
3. 是递归的
4. 返回值不是void，却可能不执行return语句
5. 包含switch/goto

inline同时也具有如下缺点：

1. 占用更多寄存器
2. 编译而成的可执行文件比较大
  - 对于一些嵌入式系统，空间更重要
  - 可能造成颠簸（thrashing，如果你还记得是什么的话）
3. 降低指令缓存的命中率（还记得指令缓存是什么吗）
4. 加入A模块调用了B模块的inline函数，那么只要该函数改动了，A模块也必须和B模块一起重新编译

定义在结构体内部的函数自带inline属性（虽然不知道为啥）

## 友元函数/友元类

1. 友元关系不能被继承
2. 别记反了：如果想让A成为B的朋友，那么应该在B里写`friend A`，然后A就可以访问B的private和protected成员

## 运算符重载

1. 不能被重载的运算符：`.  ::  ?:  sizeof`
2. 重载过的`&&  ||`不再具有**短路**性质
3. 流IO`>>  <<`的重载：
```c++
ostream& operator<<(ostream& os, const T& obj) {
	os << obj.a << obj.b << endl;
	return os
}
istream& operator>>(istream& is, T& obj) {
	is >> obj.a >> obj.b;
	return is;
}
```
4. 如果用户自定义的类重载了`()`运算符，它就会成为一个**函数对象(function object/functor)**
5. 自增自减运算符`++ --`的重载，注意参数和返回类型的区别
```cpp
struct X {
	int a;
	// ++X
	X& operator++() {
		a++;
		return *this;
	}
	// X++
	X operator++(int) {
		X tmp(*this); // copy
		operator++(); // a++
		return tmp;   // return old value
	}
};
```
6. 赋值运算符`=`可以用来实现deep copy，当类成员有指针的时候比较有用。**Copy Constructor**也是一样（见下）

## 构造函数

### 默认构造函数(Default Constructor)

没有参数（有默认值的参数也算参数）的构造函数，就是默认构造函数。如果用户不定义它，编译器会自动生成一个。

### 拷贝构造函数(Copy Constructor)

1. 调用时机：对象作为函数参数、函数返回值、被赋的值时
2. 设成private会禁止拷贝该类对象
3. 参数**必须**是引用，否则会陷入“为了拷贝，必先拷贝”的循环
4. 参数**应该**是const，[原因](https://www.geeksforgeeks.org/copy-constructor-argument-const/)

### 调用顺序

1. 先基类，后派生类
2. 先成员，后整体
3. 多重继承：按基类列表的顺序
4. 设基类有构造函数B(...)，派生类有构造函数D(...)。如果不像下面这样指定B，那么B不会被调用
```cpp
D(...) : B(...) {
	<code>
}
```

## 静态成员

1. `static`的成员函数没有`this`指针
2. `static`的成员函数不能是`virtual`的
3. `static`的成员函数不能是`const`和/或`volatile`的

## 虚继承

定义不难，理解为什么需要虚继承也不难，但还有一个问题——[虚继承一定是最好的吗？](https://stackoverflow.com/questions/21558/in-c-what-is-a-virtual-base-class/21607#21607)

答案太绕了，脑壳疼，略。