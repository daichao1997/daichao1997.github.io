## 跟着OSDev网站搭建个人操作系统（一）

发现一个宝藏网站：osdev.org（我没火星吧），这是我见过的内容最丰富最全面的操作系统开发者社区，里面的wiki不仅教你从零开始搭建内核与操作系统，还传授了一些人生经验，给刚准备起航的newbie指路（或是劝退），可以说是OS新手开发者的福地了。我今天也没干什么，就按照它给的教程一步步走，在QEMU上跑通了一个空空如也的内核，然后准备把大致的步骤记录在这里。当然，我现在是个纯newbie，最多只能做个概括+翻译而已。如果你有兴趣，可以直接去读原文，我会把每篇文章的链接附上。

### [Introduction](https://wiki.osdev.org/Introduction)

欢迎加入操作系统的开发中来。这里是编程之巅，但你并非孤身一人。我们建立OSDev网站的目的不仅是为了磨练编程技术，更是为了创建社区和结交朋友。

（介绍了操作系统、内核、shell、GUI的概念，略）

为什么要开发一个操作系统？主要有四个原因：

- 从裸机上建立自己的操作系统，能让你拥有对机器的100%的控制权，这是一件很有成就感的事情
- 科学研究
- 取代现有的操作系统
- 好玩。底层编程很困难，但也十分有趣，因为它能让你对程序执行的每一个细节都了如指掌

### [Required Knowledge](https://wiki.osdev.org/Required_Knowledge)

这里列举了开发OS所需的各种技能，并强调说千万别以为自己随随便便就能完成这项任务。（当然，我自己就不具备里面的很多技能）

### [Beginner Mistakes](https://wiki.osdev.org/Beginner_Mistakes)

这篇文章大部分内容都在劝退，估计是作者见得多了，觉得与其给新手们毫无保留的鼓励，不如先泼盆冷水。作者有几段话很有意思，我给大家翻译翻译：

> ### A Hard Truth 
>
> ***No one who isn't already a seasoned developer with years of experience in several languages and environments should even be considering OS Dev yet. A decade of programming, including a few years of low-level coding in assembly language and/or a systems language such as C, is pretty much the minimum necessary to even understand the topic well enough to work in it.***

> ### 一个不太好接受的事实
>
> 如果你不是掌握多种语言、熟悉多种环境和拥有多年开发经验的老鸟，那你根本不用想着去开发一个操作系统。光是要理解这个话题并上手，你就得先修炼十年，而且还得是底层汇编与系统语言（例如C语言）兼修。

> Oh, and for the record, Linus Torvalds wasn't quite one of them--he was a graduate student when he wrote the Linux kernel and had been coding in C for years. While he was well short of that ten year mark, as a grad student who had turned his hobby into his master's thesis, he had more time on his hands to work on the project than most people would. In any case, the 'Linux 0.0.1' release he famously posted to USENET in 1991 was little more than a round-robin scheduler, nowhere close to a full system. Getting to that point took him a year. Get the picture?

> 哦还有，据记载，Linus Torvalds（Linux之父）并不是其中之一（指前文凭一己之力开发了整个操作系统的人物）。他写Linux内核的时候已经是研究生了，写了好几年的C语言。虽然不到十年，但是他把这个业余爱好写成了毕业论文，因此他有比绝大部分人都充足的时间花在这上面。无论如何，他1991年发布在USENET上的著名的Linux 0.0.1不过是在round-robin调度器的基础上稍微加了点东西而已，根本不能算一个完整的系统，而即使是这样也花了他整整一年的时间。你懂我意思吧？

当然，来都来了，我不可能因为这个就被劝退，所以继续吧。

### [Getting Started](https://wiki.osdev.org/Getting_Started)

这里给了一些比较general的建议，包括制定开发计划、准备开发环境、用GitHub托管代码等等，但对我来说没什么用，毕竟我只是图一乐，只想学点东西而已。

### [GCC Cross-Compiler](https://wiki.osdev.org/GCC_Cross-Compiler)

本文教你安装GCC交叉编译器。所谓交叉编译器，就是让你在A平台(host)上编译，B平台(target)上运行。交叉编译器是OS开发必不可少的工具，毕竟你写的OS必须脱离host平台运行，而直接用host平台的原生编译器会让你的程序里掺杂host平台的库和头文件，以及其他host平台专用的东西。所以，我们可以选用i686-elf-gcc，它编译出的i686-elf程序是符合System V ABI标准的，它可以让你更方便地上手，因为该标准下还有很多配套的工具链可以使用（例如Binutils、GCC）。

接下来我们会用到很多工具，其中一些需要我们自己从源码编译和安装。下载源码时，建议先从[清华大学开源软件镜像站的GNU FTP镜像](https://mirrors.tuna.tsinghua.edu.cn/gnu/)开始找，然后是GitHub。GNU FTP真的不行，我下载时只有5KB/s的速度。下面是编译GCC所需的环境或依赖包：

- 类Unix环境
- 内存和磁盘空间（看情况，256 MB也许不太够）
- GCC & G++，或其他系统自带的编译器
- Make, Bison, Flex, GMP, MPFR, MPC, Texinfo
- ISL (optional), CLooG (optional)

我下载了gcc-9.3.1 & binutils-2.34 & libiconv-1.16的源码，其他则通过[Homebrew](https://brew.sh)安装。顺便一提，我使用的是MacBook Pro 2017，操作系统是macOS Mojave 10.14.6，x86_64平台。之所以要下载libiconv，是因为原文提到macOS系统自带的libiconv严重过时，需要自行更新，更新方法是将其源码拷贝至GCC的根目录下，命名为libiconv，之后编译GCC时会自动找到并使用它。

现在设置一些环境变量：

```bash
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH" # 可以写入~/.bashrc以便之后使用
```

然后新建一个文件夹，存放以上这些代码：

```bash
mkdir ~/src
mv ~/Downloads/gcc-9 ~/src
mv ~/Downloads/binutils-2.34 ~/src
```

编译并安装Binutils（一些选项的含义见原文，下同）：

```bash
mkdir build-binutils
cd build-binutils
../binutils-2.34/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
```

编译并安装GCC：

```bash
# The $PREFIX/bin dir must be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

cd ~/src
mkdir build-gcc
cd build-gcc
../gcc-9/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
```

很简单对吧？如果没问题，你现在已经可以执行`i686-elf-gcc --version`并得到正确输出了。下一篇我们将介绍如何~~通过复制粘贴~~编写一个可在虚拟环境下运行的内核。

