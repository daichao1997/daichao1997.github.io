## 跟着OSDev学习搭建操作系统（四）

上次我们编译并成功运行了myOS，这次我们来仔细看看这个简单的操作系统有哪些构成要素，要以怎样的方法去安装它。

### 编译环境的建立

源代码的编译是靠shell脚本而不是Makefile总控的，不过这并不影响我们学习。大致来讲：

- config.sh定义一大堆环境变量

- headers.sh在每个`$PROJECT`下执行`$MAKE install-headers`

- build.sh在每个`$PROJECT`下执行`$MAKE install`

所谓`$PROJECT`就是libc和kernel这两个文件夹，而`install-headers`和`install`做的事情需要到Makefile里具体看一看。

首先看libc下面的Makefile，我们挑一部分讲。

```makefile
DEFAULT_HOST!=../default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)
```

- default-host.sh会打印`i686-elf`

- target-triplet-to-arch.sh会判断输入的参数是否包含`i[[:digit]]86-`，若满足则打印`i386`，否则打印参数的开头部分，直到遇见字母、数字、下划线以外的字符（例如`x86_64`)

所以默认情况下，`$DEFAULT_HOST`与`$HOST`的值为`i686-elf`，而`$HOSTARCH`等于`i386`。绕了一大圈就设置几个字符串，何必呢？

然后是这些编译用的flag，别的都好说，但重要的是这个`-ffreestanding`选项：

```bash
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_libc -Iinclude
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk
```

####`-ffreestanding`选项的含义
这个选项我们在Bare Bone里就见过了，但当时说得比较模糊，现在我们对着GCC的文档来看：该选项将编译目标指定为“独立环境”。什么是独立环境呢？

参考：https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html#C-Dialect-Options

> Assert that compilation targets a freestanding environment. This implies `-fno-builtin`. A freestanding environment is one in which the standard library may not exist, and program startup may not necessarily be at `main`. The most obvious example is an OS kernel. This is equivalent to `-fno-hosted`.

#### Freestanding Environment

参考：https://gcc.gnu.org/onlinedocs/gcc/Standards.html#Standards

> The ISO C standard defines (in clause 4) two classes of conforming implementation. A conforming hosted implementation supports the whole standard including all the library facilities; a conforming freestanding implementation is only required to provide certain library facilities: those in `<float.h>`, `<limits.h>`, `<stdarg.h>`, and `<stddef.h>`; since AMD1, also those in `<iso646.h>`; since C99, also those in `<stdbool.h>` and `<stdint.h>`; and since C11, also those in `<stdalign.h>` and `<stdnoreturn.h>`. In addition, complex types, added in C99, are not required for freestanding implementations.

上文说，C标准规定了C编译器可以有两种实现方式，（个人译作）宿主实现和独立实现。前者要能够提供完整的C标准库，后者则只用提供特定的头文件，且不同的C版本有不同的要求。

> The standard also defines two environments for programs, a freestanding environment, required of all implementations and which may not have library facilities beyond those required of freestanding implementations, where the handling of program startup and termination are implementation-defined; and a hosted environment, which is not required, in which all the library facilities are provided and startup is through a function `int main (void)` or `int main (int, char *[])`. An OS kernel is an example of a program running in a freestanding environment; a program using the facilities of an operating system is an example of a program running in a hosted environment.

上文说，C标准也定义了两种程序运行环境：独立环境和宿主环境。独立环境是任何C编译器必须支持的，而且程序的启动与终止也可以随实现方式不同而不同。宿主环境不是C编译器所必需的，它要求提供完整的C标准库，且程序必须通过`main`函数启动。

> GCC aims towards being usable as a conforming freestanding implementation, or as the compiler for a conforming hosted implementation. By default, it acts as the compiler for a hosted implementation, defining `__STDC_HOSTED__` as 1 and presuming that when the names of ISO C functions are used, they have the semantics defined in the standard. To make it act as a conforming freestanding implementation for a freestanding environment, use the option `-ffreestanding`; it then defines `__STDC_HOSTED__` to 0 and does not make assumptions about the meanings of function names from the standard library, with exceptions noted below. To build an OS kernel, you may well still need to make your own arrangements for linking and startup. See Options Controlling C Dialect.

上文说，GCC希望能同时提供上述两种实现。默认情况下它使用“宿主实现”，但如果提供了`-ffreestanding`选项，它将不会从C标准库里寻找函数名（除了下面的特例）。

> GCC does not provide the library facilities required only of hosted implementations, nor yet all the facilities required by C99 of freestanding implementations on all platforms. To use the facilities of a hosted environment, you need to find them elsewhere (for example, in the GNU C library). See Standard Libraries.
>
> Most of the compiler support routines used by GCC are present in libgcc, but there are a few exceptions. GCC requires the freestanding environment provide `memcpy`, `memmove`, `memset` and `memcmp`. Finally, if `__builtin_trap` is used, and the target does not implement the trap pattern, then GCC emits a call to abort.

上文说，GCC并不在所有平台上都自带“宿主实现”专用的库，或者100%完整地自带“独立实现”要用的库。如果你要用“宿主实现”的库，就得看操作系统有没有自带，或者去GNU C library里面找。`libgcc`提供了编译器需要的大部分东西，但也有例外，例如GCC需要独立环境提供`memcpy`、`memmove`、`memset`和`memcmp`的实现。

#### 关于C标准库的一点补充

参考：https://gcc.gnu.org/onlinedocs/gcc/Standard-Libraries.html#Standard-Libraries

>GCC by itself attempts to be a conforming freestanding implementation. See [Language Standards Supported by GCC](https://gcc.gnu.org/onlinedocs/gcc/Standards.html#Standards), for details of what this means. Beyond the library facilities required of such an implementation, the rest of the C library is supplied by the vendor of the operating system. If that C library doesn’t conform to the C standards, then your programs might get warnings (especially when using -Wall) that you don’t expect.
>
>For example, the `sprintf` function on SunOS 4.1.3 returns `char *` while the C standard says that `sprintf` returns an `int`. The `fixincludes` program could make the prototype for this function match the Standard, but that would be wrong, since the function will still return `char *`.
>
>If you need a Standard compliant library, then you need to find one, as GCC does not provide one. The GNU C library (called `glibc`) provides ISO C, POSIX, BSD, SystemV and X/Open compatibility for GNU/Linux and HURD-based GNU systems; no recent version of it supports other systems, though some very old versions did. Version 2.2 of the GNU C library includes nearly complete C99 support. You could also ask your operating system vendor if newer libraries are available.

GCC会尽量去满足独立实现，但C函数库的剩余部分就需要操作系统的供应商去提供。如果你的库不符合C标准，那么编译程序时可能会产生大量warning，例如Sun OS 4.1.3的`sprintf`函数会返回`char*`，但C标准要求返回`int`。

如果你需要符合标准的库，你得自己去找，因为GCC并不提供。GNU C library（又称“glibc”）为一些系统提供了一些标准的支持，你也可以去问问供应商是否有更新它们的库。