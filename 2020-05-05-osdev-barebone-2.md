## 跟着OSDev网站搭建个人操作系统（二）

上回我们讲（翻译）了一些预备知识，并准备好了交叉编译器和其他工具链，现在开始动手写代码吧！我们的“空内核”需要三个源文件：

- boot.S，作为内核的入口，用于初始化运行环境
- kernel.c，作为内核的主函数
- linker.ld，作为链接上面两个文件的脚本

### [Booting the Operating System](https://wiki.osdev.org/Bare_Bones#Booting_the_Operating_System)

当你编译好一个内核后，它是存储于磁盘上的，然而一个没有内核的机器该如何把磁盘上的内核读进内存，从而去运行里面的指令呢？这就要用到一个内核之外的东西，叫做**bootloader**。原文提到了GNU有一个叫做[GRUB](https://wiki.osdev.org/GRUB)的现成工具直接用。

那么为什么推荐用GRUB，而不是自己写一个呢？说好的“自己动手”呢？这里就要提到由GNU提出的[操作系统多重引导规范（Multiboot Specification）](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)。此规范针对的是这样一个问题——操作系统有茫茫多，平台架构也有不少，而bootloader是同时取决于这两者的，因为它既要初始化平台环境，又要装载内核。假如大家随意发挥，那么当我在PC上运行其他内核时，就可能出现冲突，因为其他内核的bootloader不一定支持PC。因此，Multiboot Specification同时对bootloader和内核作了约束，使符合规范的bootloader能装载任何符合规范的内核。GRUB就是这样的bootloader，而且它还具备其他特性，例如可配置等。

可能有人会问：bootloader又是怎么被读进内存的呢？实际上bootloader并不需要其他程序去读取，因为它存在于一个特殊的固件里，叫做[BIOS (Basic Input/Output System)](https://en.wikipedia.org/wiki/BIOS)。电脑一通上电，最先做的事情之一就是去执行BIOS里的指令，这是出厂时就预设好的。

#### [Installing GRUB 2 on OS X](https://wiki.osdev.org/GRUB#Installing_GRUB_2_on_OS_X)

1. 首先你需要一个交叉编译器及其目标平台的工具链（这个在前篇就完成了），然后安装[objconv](https://github.com/vertis/objconv)。如果后面遇到了关于“aclocal”的错，你需要安装automake。

2. 下载grub的源码，编译并安装。注意我的下载地址，[参考这里](https://github.com/intermezzOS/book/issues/200)。

```bash
git clone git@github.com:ar-OS/grub.git --depth=1
mkdir build-grub
cd build-grub
../grub/configure --disable-werror TARGET_CC=i686-elf-gcc TARGET_OBJCOPY=i686-elf-objcopy TARGET_STRIP=i686-elf-strip TARGET_NM=i686-elf-nm TARGET_RANLIB=i686-elf-ranlib --target=i686-elf
make
make install
```

#### [Bootstrap Assembly](https://wiki.osdev.org/Bare_Bones#Bootstrap_Assembly)

首先创建一份boot.S代码，下面我把原文里的注释稍微概括一下。

```asm
/* 多重引导规范所要求的一些常量 */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/* 
声明一个符合“多重引导规范”的头部，让bootloader能找到并确认这里是内核的开头
bootloader会在内核的前8KiB里去寻找这些内容
*/
.section .multiboot /* 单独起一个程序段，确保它能在内核的最开头出现 */
.align 4            /* 要求32-bit对齐 */
.long MAGIC
.long FLAGS
.long CHECKSUM
 
/*
内核自己起一个内核栈，并用符号标明栈顶和栈底的位置
x86的栈是从栈顶向栈底生长的
*/
.section .bss /* BSS段不占据程序空间，而是在装载时由装载程序分配空间 */
.align 16     /* 一定要对齐，否则会产生未定义行为 */
stack_bottom:
.skip 16384 # 16 KiB
stack_top:
 
.section .text
.global _start /* 链接脚本会将这里指定为程序入口，bootloader装载完内核后会跳转到这里，并且再也不返回 */
.type _start, @function
_start:
	/*
	bootloader会开启x86的32-bit保护模式，同时关闭中断和分页，
	将CPU设置为Multiboot规范所要求的状态。此时没有printf函数，
	没有安全级别限制，也没有debug机制，只有内核自己。
  此时内核拥有对机器的完全控制权。
	*/
 
	/*
	设置esp寄存器。只能用汇编语言完成，
	因为C语言程序的运行必须依赖栈，而此时还没有栈
	*/
	mov $stack_top, %esp
 
	/*
	在进入内核的更上层之前，我们最好先初始化一些关键的CPU状态。
	浮点运算指令、扩展指令集都还没有启用。
	应该在这里装载GDT、开启分页。
  It's best to minimize the early environment where crucial features are offline. 
	C++ features such as global constructors and exceptions will require runtime support to work as well.
	*/
 
	/*
	进入内核的更上一层。根据System V ABI的要求，执行call指令时，
	栈必须是16-byte对齐的，现在我们满足这个要求
	*/
	call kernel_main
 
	/*
	如果系统已经没事做了，就进入无限循环：
	1. 用cli指令禁止中断，我们在bootloader已经做过了。不过
	   你可能还会在kernel_main里打开中断并返回这里，所以还
	   是要cli一次。当然，从kernel_main返回这件事本身就挺扯的。
	2. 用hlt指令锁住电脑，直到下一次中断来临。
	3. 由于我们已经关掉了中断，所以只有不可屏蔽中断和x86的
	   “系统管理模式”会唤醒电脑。万一这种情况发生了，我们再跳回hlt指令。
	*/
	cli
1:	hlt
	jmp 1b
 
/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start
```

附：[系统管理模式 - 维基百科](https://en.wikipedia.org/wiki/System_Management_Mode)

### [Implementing the Kernel](https://wiki.osdev.org/Bare_Bones#Implementing_the_Kernel)

#### 独立环境和宿主环境

我们平时在用户空间写C/C++程序时，都处于“宿主环境”下，其重要特征之一就是C标准库的可用性。然而现在我们处于一个“独立环境”，只有一个交叉编译器，因此我们只能在程序里include非常非常基础的头文件，例如编译器自带的`<stdbool.h>`（定义了布尔类型）、 `<stddef.h>`（定义了`size_t`和`NULL`）、 `<stdint.h>`（定义了定长的数据类型`intx_t`和`uintx_t` ，这对操作系统编程非常重要。万一哪天`short`的长度就变了呢？）。此外还有 <float.h>、<iso646.h>、<limits.h>、<stdarg.h>等头文件。

下面的kernel.c非常self-explanatory，我就不多解释了，大意就是。注意缺少C标准库是一件多么麻烦的事情，我们得自己实现`strlen`函数。还有，很多新机器已经不支持VGA文字模式（以及BIOS）了，而是转向了[UEFI](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface)，而在后者的情况下你甚至需要自己设计每个字符的像素buffer。

#### 用C语言写一个内核

```c
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
 
/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};
 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000; // VGA默认的彩色显示器缓存地址，可以去查查这个数
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c) {
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}
 
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
 
void kernel_main(void) {
	/* Initialize terminal interface */
	terminal_initialize();
 
	/* Newline support is left as an exercise. */
	terminal_writestring("Hello, kernel World!\n");
}
```

### [Linking_the_Kernel](https://wiki.osdev.org/Bare_Bones#Linking_the_Kernel)

在用户空间下编程时，GCC可以用自带的脚本自动链接多个object文件，但是系统编程时不要这么做。我们要写一个自己的链接脚本linker.ld。你可能需要先了解一下程序链接的知识，例如程序段、链接地址、加载地址的概念。还可以参考[ld的文档](http://www.scoberlin.de/content/media/http/informatik/gcc_docs/ld_toc.html#TOC5)来学习编写链接脚本：

```c
/* bootloader会从_start处进入内核 */
ENTRY(_start)
 
/* 指定内核每个程序段的链接地址、加载地址等 */
SECTIONS
{
	/* 从1MiB这个地址开始，因为bootloader一般都是把内核加载到这里的 */
	. = 1M;
 
	/* 把multiboot段放在最前面 */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}
 
	/* 只读数据 */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
 
	/* 已初始化的可读写数据 */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
 
	/* 未初始化的可读写数据（包括栈） */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
 
	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}
```

### 编译、链接成内核镜像

```bash
i686-elf-as boot.s -o boot.o
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
```

你应该可以看到上面的程序合成了一个内核镜像`myos.bin`。

### 运行你的内核！

- 安装xorriso，安装QEMU

- 把下面的代码保存为grub.cfg文件。

```
menuentry "myos" {
	multiboot /boot/myos.bin
}
```

- 把myos.bin通过`grub-mkrescue`打包成GRUB能直接用的CD-ROM镜像文件myos.iso

```bash
mkdir -p isodir/boot/grub
cp myos.bin isodir/boot/myos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o myos.iso isodir
```

- 虚拟环境启动：运行`qemu-system-i386 -cdrom myos.iso`或者`qemu-system-i386 -kernel myos.bin`，都能启动内核，但可以看出前者调出了GRUB的图形界面，而后者没有，且存在一些小bug。
- 真实环境启动：把myos.iso烧录至你的U盘、光盘或磁盘，然后插在电脑上，选择用外部存储介质启动！（我没敢试，哈哈）

（全文完）