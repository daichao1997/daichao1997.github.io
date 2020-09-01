## 《算法导论》的翻译真叫一个垃圾！

其实不只是这一本教材，国内对英文计算机文献的翻译质量普遍不高，以至于我直接读英文原著都要比读这种狗屁不通的汉语句子流畅许多。下面这个例子是第14章开头的第一段话，我来试试把原版翻译改良一下，你们品评一下有没有好读许多。

英文原文：

> Some engineering situations require no more than a “textbook” data structure—such as a doubly linked list, a hash table, or a binary search tree—but many others require a dash of creativity. Only in rare situations will you need to create an entirely new type of data structure, though. More often, it will suffice to augment a textbook data structure by storing additional information in it. You can then program new operations for the data structure to support the desired application. Augmenting a data structure is not always straightforward, however, since the added information must be updated and maintained by the ordinary operations on the data structure.

机械工业出版社的翻译：

> 一些工程应用需要的只是一些“教科书”中的标准数据结构，比如双链表、散列表或二叉搜索树等，然而也有许多其他的应用需要对现有数据结构进行少许地创新和改造，但是只在很少情况下需要创造出一类全新类型的数据结构。更经常的是，通过存储额外信息的方法来扩张一种标准的数据结构，编写新的操作来支持所需要的应用。然而对数据结构的扩张并不总是简单直接的，因为添加的信息必须要能被该数据结构上的常规操作更新和维护。

这个翻译的槽点实在太多了！

首先第一句“……然而……但是……”双重转折绕得人云里雾里，其实完全可以按照原文用两句话说完。

“对现有数据结构进行少许地创新和改造”，“的地得”都弄错了，而且原文就一句"a dash of creativity"，怎么被扩写得这么长？

“一类全新类型的数据结构”，不觉得累赘吗？

“更经常的是，通过XXX来YYY，YYY”，为什么不把“来”用逗号分出来，而要把“YYY”分成两句？还有，这句话的主语呢？

我自己的翻译：

> 某些工程应用只需要用“教科书式”的标准数据结构就够了，例如双链表、散列表、二叉搜索树等，然而有时候也需要我们发挥一点创造力。当然，只有在极少数情况下我们才需要从零开始创造一种全新的数据结构。一般来讲，我们只需要在标准数据结构中添加额外的信息就够了，然后你就可以为它编写新的操作来支持你的需求。然而扩张一个数据结构并不总是简单直接的，因为你还需要保证你额外添加的信息能够通过该数据结构原有的操作来更新和维护。

我个人觉得，除了少数需要严格描述的地方（如定义、证明），其他只需要表意的内容要做到简洁易懂，要让读者最快地理解你的核心观点。这一点英文版做到了，但中文翻译却把它变得又冗长又累赘又不通顺。本来中文就缺少断句用的空格，你再把一句话弄得长长的，真是在阻碍广大计算机爱好者学习新知识啊！