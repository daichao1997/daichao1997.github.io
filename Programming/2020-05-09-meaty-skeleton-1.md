## 跟着OSDev学习搭建操作系统（三）

前几天跑通了一个空空如也的内核（教程里叫做Bare Bone），只能说是搭建好了环境，接下来要搭建的myOS会稍微正式一点。如同[教程](https://wiki.osdev.org/Meaty_Skeleton)标题“Meaty Skeleton”所说的那样，myOS虽然有一点“meat”，但依然只是一具“skeleton”，没有实际功能。我们既可以从中学到一个科学的OS源代码的结构，也可以直接以myOS为基础，向里面添加新的功能。

这一篇教程的信息量要远多于上一篇，其难点不在于看懂每个函数（函数都很简单），而在于把握全局，从整体出发去体会细节。这么说可能有点抽象，但我个人感觉这是搞OS最需要培养的一种感觉。所以，本文会非常详细地记录自己遇到的每一个疑惑，并尝试去解答它们。虽然我现在离成为真正的OS开发者还有很大差距，但我相信差距一定可以一步步拉近。那么来拉取代码吧：

```bash
git clone https://gitlab.com/sortie/meaty-skeleton.git
```

### 在macOS上编译的注意事项

拿到代码后，第一步当然是自己编译试试，于是本macOS用户遇到了这个error：

```
Makefile:23: arch//make.config: No such file or directory
```

经过一番调查，发现macOS自带的make，也就是通过Xcode Command Line Tools安装的make，与GNU make并非同一版本，这会导致`!=`赋值符号不受支持：

```
> make --version # 自带make
GNU Make 3.81
> brew install make # GNU make
GNU "make" has been installed as "gmake".
> gmake
> gmake --version
GNU Make 4.3
```

解决办法很简单，要么改用`gmake	`，要么改一下PATH：

```bash
PATH="/usr/local/opt/make/libexec/gnubin:$PATH"
```

后面可能还会报一个关于`cp`的错，原因同上，我们需要安装一下GNU coreutils，感兴趣的可以参考[这个帖子](https://apple.stackexchange.com/questions/69223/how-to-replace-mac-os-x-utilities-with-gnu-core-utilities)多装一些别的：

```
brew install coreutils
```

不过这样安装之后GNU cp会以`gcp`的名字存在，所以可以再改一下PATH：

```
PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"
```

其他GNU命令都可以用类似的办法取代macOS版本的命令。

### 编译结果的初步观察

运行`./build.sh`，可以看到生成了两个文件夹isodir与sysroot，前者与我们在Bare Bone里看到的一样，是给GRUB用来启动的，后者则有下属文件夹boot与usr。boot里装的myos.kernel我们也见过，它就是编译与链接后形成的内核本身，但多出来的usr文件夹是什么呢？

我们知道，“操作系统”与“内核”是前者包含后者的关系。操作系统除了内核，还应该提供给用户一些交互工具，如命令行、编译器、文本编辑器、软件包管理工具等。如果你用过GNU/Linux系统，你一定见过根目录下诸如/usr/bin之类的文件夹，里面装着各种二进制文件，这些就是留给用户使用的。myOS也是一样，虽然现在我们什么都没有，但起码可以给用户留一个文件夹，里面放一些最简单的头文件等内容。类似地，你也可以在GNU/Linux系统里找到/boot文件夹，里面的东西也是与内核本身相关的东西，不过比myOS丰富得多。

总而言之，sysroot文件夹是myOS的安装目的地，同时也是myOS用户的“根目录“。OS编译完成的同时，这个文件夹也成为了一个合格的启动目录。如果我们新开一个磁盘分区，把它复制过去，再从该分区启动，就会发生这些事情：bootloader会将内核读进内存，内核里的硬盘驱动与文件系统驱动又会把剩下的文件读进根目录（实际过程可以非常复杂，参考GNU/Linux系统的[initrd](https://en.wikipedia.org/wiki/Initial_ramdisk)），最后形成用户看到的完整的根目录。
